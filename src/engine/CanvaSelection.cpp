#include "Canva.hpp"

namespace vipe {

GdkPixbuf* mat_to_pixbuf(const cv::Mat& mat) {
    cv::Mat mat_converted;

    if (mat.channels() == 1) {
        cv::cvtColor(mat, mat_converted, cv::COLOR_GRAY2RGB);
    } else if (mat.channels() == 4) {
        cv::cvtColor(mat, mat_converted, cv::COLOR_BGRA2RGBA);
    } else if (mat.channels() == 3) {
        cv::cvtColor(mat, mat_converted, cv::COLOR_BGR2RGB);
    } else {
        g_warning("Unsupported number of channels: %d", mat.channels());
        return nullptr;
    }
    int width    = mat_converted.cols;
    int height   = mat_converted.rows;
    int channels = mat_converted.channels();
    // Taille du tampon mémoire
    int rowstride = width * channels;
    // Allouer une mémoire continue pour les données de l'image
    guchar* pixbuf_data = (guchar*)g_malloc(height * rowstride);
    // Copier les données manuellement dans la mémoire continue
    for (int y = 0; y < height; ++y) {
        memcpy(pixbuf_data + y * rowstride, mat_converted.ptr(y), rowstride);
    }
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(
        pixbuf_data, GDK_COLORSPACE_RGB, channels == 4, 8, width, height, rowstride,
        [](guchar* data, gpointer) { g_free(data); }, nullptr);
    return pixbuf;
}

void Canva::copy_selection_to_clipboard() {
    cv::Mat mat;
    if (!_selection.is_selection_active()) {
        mat = _layers[_selected_layer].image.clone();
    } else {
        mat = _selection.get_mask().clone();
    }
    GdkPixbuf*    pixbuf    = mat_to_pixbuf(mat);
    GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_image(clipboard, pixbuf);
    g_object_unref(pixbuf);
}

void Canva::cut_selection_to_clipboard() {
    copy_selection_to_clipboard();
    _selection.clear();
}

static cv::Mat pixbuf_to_mat(GdkPixbuf* pixbuf) {
    int     width    = gdk_pixbuf_get_width(pixbuf);
    int     height   = gdk_pixbuf_get_height(pixbuf);
    int     channels = gdk_pixbuf_get_n_channels(pixbuf);
    int     stride   = gdk_pixbuf_get_rowstride(pixbuf);
    guchar* pixels   = gdk_pixbuf_get_pixels(pixbuf);

    cv::Mat mat;
    if (channels == 3) {
        mat = cv::Mat(height, width, CV_8UC3, pixels, stride);
        cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR); // Conversion RGB -> BGR
    } else if (channels == 4) {
        mat = cv::Mat(height, width, CV_8UC4, pixels, stride);
        cv::cvtColor(mat, mat, cv::COLOR_RGBA2BGRA); // Conversion RGBA -> BGRA
    }
    return mat.clone();
}

void Canva::paste_from_clipboard() {
    GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    GdkPixbuf*    pixbuf    = gtk_clipboard_wait_for_image(clipboard);

    if (pixbuf) {
        cv::Mat mat = pixbuf_to_mat(pixbuf);
        if (mat.empty()) {
            g_warning("Failed to convert pixbuf to mat!");
            return;
        }
        cv::Mat& layer_image = _layers[_selected_layer].image;
        if (layer_image.channels() == 3 && mat.channels() == 1) {
            cv::cvtColor(mat, mat, cv::COLOR_GRAY2BGR);
        } else if (layer_image.channels() == 1 && mat.channels() == 3) {
            cv::cvtColor(mat, mat, cv::COLOR_BGR2GRAY);
        } else if (layer_image.channels() == 4 && mat.channels() == 3) {
            cv::cvtColor(mat, mat, cv::COLOR_RGB2RGBA);
        } else if (layer_image.channels() == 3 && mat.channels() == 4) {
            cv::cvtColor(mat, mat, cv::COLOR_RGBA2RGB);
        }
        emplace_selection();
        _selection.clear();
        _selection.set_mask(mat);
        _selection.set_start(cv::Point(0, 0));
        _selection.set_end(cv::Point(mat.cols, mat.rows));
        _selection.set_type(0);
        update_selected_region();
        g_object_unref(pixbuf);
    } else {
        g_warning("No image in clipboard!");
    }
}

void Canva::render_selection(cv::Mat& canvas) {
    if (!_selection.is_selection_active() || _selection.get_mask().empty()) {
        return;
    }

    const cv::Mat& selection_mask = _selection.get_mask();
    cv::Rect       render_region  = _selected_region;

    int render_x      = render_region.x * _zoom_factor;
    int render_y      = render_region.y * _zoom_factor;
    int render_width  = render_region.width * _zoom_factor;
    int render_height = render_region.height * _zoom_factor;

    int visible_x      = std::max(0, render_x);
    int visible_y      = std::max(0, render_y);
    int visible_width  = std::min(render_width - (visible_x - render_x), canvas.cols - visible_x);
    int visible_height = std::min(render_height - (visible_y - render_y), canvas.rows - visible_y);

    if (visible_width <= 0 || visible_height <= 0) {
        return;
    }

    int mask_x      = std::max(0, -render_region.x);
    int mask_y      = std::max(0, -render_region.y);
    int mask_width  = visible_width / _zoom_factor;
    int mask_height = visible_height / _zoom_factor;

    cv::Rect mask_region(mask_x, mask_y, mask_width, mask_height);
    if (mask_region.width <= 0 || mask_region.height <= 0) {
        return;
    }

    cv::Mat visible_mask = selection_mask(mask_region);

    cv::Mat resized_mask;
    cv::resize(visible_mask, resized_mask, cv::Size(visible_width, visible_height), 0, 0,
               cv::INTER_NEAREST);

    cv::Rect canvas_region(visible_x, visible_y, visible_width, visible_height);
    resized_mask.copyTo(canvas(canvas_region));
}

void Canva::draw_selection_rect(const Cairo::RefPtr<Cairo::Context>& cr) {
    if (!_selection.is_selection_active())
        return;
    if (_selection.get_type() == Selection::Type::Rectangular) {
        auto   start  = _selection.get_start();
        auto   end    = _selection.get_end();
        double x      = std::min(start.x, end.x);
        double y      = std::min(start.y, end.y);
        double width  = std::abs(end.x - start.x);
        double height = std::abs(end.y - start.y);

        cr->set_source_rgba(0, 0.4, 0.7, 0.2);
        cr->rectangle(x * _zoom_factor + _view_offset_x, y * _zoom_factor + _view_offset_y,
                      width * _zoom_factor, height * _zoom_factor);
        cr->fill();

        std::vector<double> dashes = {5.0, 5.0};
        cr->set_dash(dashes, 0);
        cr->set_source_rgba(0, 0, 0, 1.0);
        cr->rectangle(x * _zoom_factor + _view_offset_x, y * _zoom_factor + _view_offset_y,
                      width * _zoom_factor, height * _zoom_factor);
        cr->stroke();
        auto handles = _selection.get_resize_handles_points(cv::Rect(x, y, width, height));
        for (const auto& handle : handles) {
            cr->set_source_rgba(0.7, 0.7, 0.7, 1.0);
            cr->rectangle(handle.x * _zoom_factor + _view_offset_x,
                          handle.y * _zoom_factor + _view_offset_y, handle.width * _zoom_factor,
                          handle.height * _zoom_factor);
            cr->fill();
            cr->set_source_rgba(1, 1, 1, 0.7);
            cr->rectangle(handle.x * _zoom_factor + _view_offset_x,
                          handle.y * _zoom_factor + _view_offset_y, handle.width * _zoom_factor,
                          handle.height * _zoom_factor);
            cr->stroke();
        }
    }
}

bool Canva::selection_out_of_bounds() {
    return _selected_region.x < 0 || _selected_region.y < 0 ||
           _selected_region.x + _selected_region.width > _image.cols ||
           _selected_region.y + _selected_region.height > _image.rows;
}

void Canva::empty_selection_on_layer() {
    if (!_selection.is_selection_active()) {
        return;
    }
    auto& layer = _layers[_selected_layer].image;
    if (layer.empty()) {
        return;
    }
    cv::Mat region_content = layer(_selected_region).clone();
    _selection.set_mask(region_content);
    layer(_selected_region) = cv::Scalar(0, 0, 0, 0);
    mark_layer_for_update(_selected_layer);
}

bool Canva::init_resize_selection(int x, int y) {
    if (!_selection.is_selection_active())
        return false;
    apply_canva_drawing_factors(x, y);
    auto handles = _selection.detect_resize_handle(x, y);
    if (handles == Selection::ResizeHandle::None)
        return false;
    _selection.set_active_handle(handles);
    _prev_x = x;
    _prev_y = y;
    return true;
}

void Canva::resize_selection_from_handle(int x, int y) {
    apply_canva_drawing_factors(x, y);
    int dx = x - _prev_x;
    int dy = y - _prev_y;

    cv::Point old_start = _selection.get_start();
    cv::Point old_end   = _selection.get_end();
    cv::Mat   old_mask  = _selection.get_mask().clone();

    switch (_selection.get_active_handle()) {
    case Selection::ResizeHandle::TopLeft:
        old_start.x += dx;
        old_start.y += dy;
        break;
    case Selection::ResizeHandle::TopRight:
        old_end.x += dx;
        old_start.y += dy;
        break;
    case Selection::ResizeHandle::BottomLeft:
        old_start.x += dx;
        old_end.y += dy;
        break;
    case Selection::ResizeHandle::BottomRight:
        old_end.x += dx;
        old_end.y += dy;
        break;
    default:
        return;
    }

    bool flipped_x = old_start.x > old_end.x;
    bool flipped_y = old_start.y > old_end.y;

    if (flipped_x)
        std::swap(old_start.x, old_end.x);
    if (flipped_y)
        std::swap(old_start.y, old_end.y);

    int min_size = 5;
    if (old_end.x - old_start.x < min_size)
        old_end.x = old_start.x + min_size;
    if (old_end.y - old_start.y < min_size)
        old_end.y = old_start.y + min_size;

    _selection.set_start(old_start);
    _selection.set_end(old_end);

    if (!old_mask.empty()) {
        cv::Size new_size(old_end.x - old_start.x, old_end.y - old_start.y);
        cv::Mat  resized_mask;

        if (new_size.width > 0 && new_size.height > 0) {
            cv::resize(old_mask, resized_mask, new_size, 0, 0, cv::INTER_LINEAR);
            _selection.set_mask(resized_mask);
        }
    }

    _prev_x = x;
    _prev_y = y;
    update_selected_region();
}

void Canva::set_selection_start(int x, int y, int type) {
    emplace_selection();
    _selection.clear();
    update_selected_region();
    _selection.set_type(type);
    apply_canva_drawing_factors(x, y);
    x = std::clamp(x, 0, _image.cols);
    y = std::clamp(y, 0, _image.rows);
    _selection.set_start(cv::Point(x, y));
}

void Canva::resize_selection(int x, int y, int type) {
    apply_canva_drawing_factors(x, y);
    x = std::clamp(x, 0, _image.cols);
    y = std::clamp(y, 0, _image.rows);
    if (type == 2)
        _selection.add_point(cv::Point(x, y));
    else if (type == 3)
        return;
    else
        _selection.set_end(cv::Point(x, y));
}

void Canva::set_selection_end(int x, int y, int type) {
    // type = 2 == lasso, type = 3 == magic_wand
    apply_canva_drawing_factors(x, y);
    x = std::clamp(x, 0, _image.cols);
    y = std::clamp(y, 0, _image.rows);
    if (type == 2)
        _selection.add_point(cv::Point(x, y));
    else if (type == 3)
        return;
    else
        _selection.set_end(cv::Point(x, y));
    if (!_selection.is_selection_active())
        _selection.clear();
    update_selected_region();
    copy_from_layer_to_selection();
    empty_selection_on_layer();
}

void Canva::init_move_selection(int x, int y) {
    if (init_resize_selection(x, y))
        return;
    if (!_selection.is_selection_active()) {
        auto& layer = _layers[_selected_layer].image;
        if (layer.empty()) {
            return;
        }
        _selection.set_start(cv::Point(0, 0));
        _selection.set_end(cv::Point(layer.cols, layer.rows));
        _selection.set_mask(layer.clone());
        _selection.set_type(0);
        empty_selection_on_layer();
    }

    apply_canva_drawing_factors(x, y);
    _prev_x = x;
    _prev_y = y;
}

void Canva::move_selection(int x, int y) {
    if (_selected_region.empty() || !_selection.is_selection_active()) {
        return;
    }
    if (_selection.get_active_handle() != Selection::ResizeHandle::None) {
        resize_selection_from_handle(x, y);
        return;
    }
    apply_canva_drawing_factors(x, y);
    int offset_x = -(_prev_x - x);
    int offset_y = -(_prev_y - y);

    _selected_region.x += offset_x;
    _selected_region.y += offset_y;
    _selection.set_start(cv::Point(_selected_region.x, _selected_region.y));
    _selection.set_end(cv::Point(_selected_region.x + _selected_region.width,
                                 _selected_region.y + _selected_region.height));
    _prev_x = x;
    _prev_y = y;
}

void Canva::stop_selection_grab() { _selection.set_active_handle(Selection::ResizeHandle::None); }

void Canva::emplace_selection() {
    auto& layer = _layers[_selected_layer].image;

    if (layer.empty() || _selection.get_mask().empty() || _selected_region.empty()) {
        return;
    }
    int x_start = std::max(0, _selected_region.x);
    int y_start = std::max(0, _selected_region.y);
    int x_end   = std::min(layer.cols, _selected_region.x + _selected_region.width);
    int y_end   = std::min(layer.rows, _selected_region.y + _selected_region.height);
    if (x_end <= x_start || y_end <= y_start) {
        return;
    }
    cv::Rect valid_region(x_start, y_start, x_end - x_start, y_end - y_start);
    int      mask_x_start = x_start - _selected_region.x;
    int      mask_y_start = y_start - _selected_region.y;
    cv::Rect mask_region(mask_x_start, mask_y_start, valid_region.width, valid_region.height);
    cv::Mat  selected_area = _selection.get_mask()(mask_region);
    selected_area.copyTo(layer(valid_region));
    mark_layer_for_update(_selected_layer);
}

} // namespace vipe