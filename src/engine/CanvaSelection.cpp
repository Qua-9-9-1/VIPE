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
        _selection.create_rect(0, 0);
        _selection.set_end(cv::Point(mat.cols, mat.rows));
        _selection.set_last_rect_end(mat.cols, mat.rows);
        _selection.set_type(0);
        _selection.update_selected_region(_image);
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
    cv::Rect       render_region  = _selection.get_selected_region();

    int render_x       = render_region.x * _zoom_factor;
    int render_y       = render_region.y * _zoom_factor;
    int render_width   = render_region.width * _zoom_factor;
    int render_height  = render_region.height * _zoom_factor;
    int visible_x      = std::max(0, render_x);
    int visible_y      = std::max(0, render_y);
    int visible_width  = std::min(render_width, canvas.cols - visible_x);
    int visible_height = std::min(render_height, canvas.rows - visible_y);

    if (visible_width <= 0 || visible_height <= 0) {
        return;
    }

    int mask_x      = std::max(0, -render_region.x);
    int mask_y      = std::max(0, -render_region.y);
    int mask_width  = std::min(visible_width / _zoom_factor, double(selection_mask.cols - mask_x));
    int mask_height = std::min(visible_height / _zoom_factor, double(selection_mask.rows - mask_y));

    if (mask_width <= 0 || mask_height <= 0) {
        return;
    }

    cv::Rect mask_region(mask_x, mask_y, mask_width, mask_height);
    cv::Mat  visible_mask = selection_mask(mask_region);
    cv::Mat  selection_rgba(render_height, render_width, canvas.type(), cv::Scalar(0, 0, 0, 0));
    cv::Mat  resized_mask;
    cv::resize(visible_mask, resized_mask,
               cv::Size(mask_width * _zoom_factor, mask_height * _zoom_factor), 0, 0,
               cv::INTER_NEAREST);
    cv::Rect paste_region((mask_x * _zoom_factor), (mask_y * _zoom_factor), resized_mask.cols,
                          resized_mask.rows);
    resized_mask.copyTo(selection_rgba(paste_region));
    cv::Rect canvas_region(visible_x, visible_y, visible_width, visible_height);
    cv::Mat  roi_canvas    = canvas(canvas_region);
    cv::Mat  roi_selection = selection_rgba(
        cv::Rect(visible_x - render_x, visible_y - render_y, visible_width, visible_height));
    cv::Mat mask_alpha;
    cv::extractChannel(roi_selection, mask_alpha, 3);
    cv::Mat mask_binary = mask_alpha > 0;
    roi_selection.copyTo(roi_canvas, mask_binary);
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
        auto selections_rects = _selection.get_selections_rects();
        for (const auto& rect : selections_rects) {
            cr->rectangle(rect.x * _zoom_factor + _view_offset_x,
                          rect.y * _zoom_factor + _view_offset_y, rect.width * _zoom_factor,
                          rect.height * _zoom_factor);
            cr->fill();
        }

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
    if (!_selection.is_selection_active())
        return false;
    auto selected_region = _selection.get_selected_region();
    return selected_region.x < 0 || selected_region.y < 0 ||
           selected_region.x + selected_region.width > _image.cols ||
           selected_region.y + selected_region.height > _image.rows;
}

// void Canva::empty_selection_on_layer() {
//     if (!_selection.is_selection_active()) {
//         return;
//     }
//     auto& layer = _layers[_selected_layer].image;
//     if (layer.empty()) {
//         return;
//     }
//     auto    selected_region = _selection.get_selected_region();
//     cv::Mat region_content  = layer(selected_region).clone();
//     _selection.set_mask(region_content);
//     layer(selected_region) = cv::Scalar(0, 0, 0, 0);
//     mark_layer_for_update(_selected_layer);
// }

void Canva::empty_selection_on_layer() {
    if (!_selection.is_selection_active()) {
        return;
    }

    auto& layer = _layers[_selected_layer].image;
    if (layer.empty()) {
        return;
    }

    _selection.normalize_selection();
    _selection.update_selected_region(_image);
    auto selected_region = _selection.get_selected_region();

    // Vérifier que selected_region est valide
    if (selected_region.x < 0 || selected_region.y < 0 ||
        selected_region.x + selected_region.width > layer.cols ||
        selected_region.y + selected_region.height > layer.rows) {
        std::cerr << "Invalid selected region: " << selected_region << std::endl;
        return;
    }

    // Créer un masque vide avec la taille de la sélection
    cv::Mat mask(selected_region.height, selected_region.width, CV_8UC4, cv::Scalar(0, 0, 0, 0));

    auto selections_rects = _selection.get_selections_rects();

    for (const auto& rect : selections_rects) {
        // Ajuster `roi` pour être dans les limites de `layer`
        cv::Rect roi = rect & cv::Rect(0, 0, layer.cols, layer.rows);

        if (roi.empty())
            continue; // Ignorer si hors image

        // Ajuster `mask_roi` pour être dans les limites de `mask`
        cv::Rect mask_roi(roi.x - selected_region.x, roi.y - selected_region.y, roi.width,
                          roi.height);
        mask_roi = mask_roi & cv::Rect(0, 0, mask.cols, mask.rows);

        if (mask_roi.empty())
            continue; // Ignorer si hors masque

        // Copier la partie du layer dans le masque
        layer(roi).copyTo(mask(mask_roi));
    }

    _selection.set_mask(mask);

    // Effacer les zones sélectionnées dans l'image
    for (const auto& rect : selections_rects) {
        cv::Rect roi = rect & cv::Rect(0, 0, layer.cols, layer.rows);
        if (roi.empty())
            continue; // Vérification

        layer(roi) = cv::Scalar(0, 0, 0, 0);
    }

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
    cv::Point new_start = old_start;
    cv::Point new_end   = old_end;
    cv::Mat   old_mask  = _selection.get_mask().clone();

    switch (_selection.get_active_handle()) {
    case Selection::ResizeHandle::TopLeft:
        new_start.x += dx;
        new_start.y += dy;
        break;
    case Selection::ResizeHandle::TopRight:
        new_end.x += dx;
        new_start.y += dy;
        break;
    case Selection::ResizeHandle::BottomLeft:
        new_start.x += dx;
        new_end.y += dy;
        break;
    case Selection::ResizeHandle::BottomRight:
        new_end.x += dx;
        new_end.y += dy;
        break;
    default:
        return;
    }

    bool flipped_x = new_start.x > new_end.x;
    bool flipped_y = new_start.y > new_end.y;

    if (flipped_x)
        std::swap(new_start.x, new_end.x);
    if (flipped_y)
        std::swap(new_start.y, new_end.y);

    int min_size = 5;
    if (new_end.x - new_start.x < min_size)
        new_end.x = new_start.x + min_size;
    if (new_end.y - new_start.y < min_size)
        new_end.y = new_start.y + min_size;

    auto selected_region = _selection.get_selected_region();
    std::cout << "Selected region: " << selected_region << std::endl;
    std::cout << "Old start: " << old_start << " Old end: " << old_end << std::endl;
    std::cout << "New start: " << new_start << " New end: " << new_end << std::endl;
    float scale_x = static_cast<float>(new_end.x - new_start.x) / selected_region.width;
    float scale_y = static_cast<float>(new_end.y - new_start.y) / selected_region.height;
    std::cout << "Scale X: " << scale_x << " Scale Y: " << scale_y << std::endl;

    _selection.apply_resize_on_rects(scale_x, scale_y, cv::Rect(new_start, new_end));

    _selection.set_start(new_start);
    _selection.set_end(new_end);

    if (!old_mask.empty()) {
        cv::Size new_size(new_end.x - new_start.x, new_end.y - new_start.y);
        cv::Mat  resized_mask;

        if (new_size.width > 0 && new_size.height > 0) {
            cv::resize(old_mask, resized_mask, new_size, 0, 0, cv::INTER_LINEAR);
            _selection.set_mask(resized_mask);
        }
    }

    _prev_x = x;
    _prev_y = y;
    _selection.update_selected_region(_image);
}

void Canva::set_selection_start(int x, int y, int type, bool multiple) {
    emplace_selection();
    _selection.set_type(type);
    apply_canva_drawing_factors(x, y);
    x = std::clamp(x, 0, _image.cols);
    y = std::clamp(y, 0, _image.rows);
    if (!multiple)
        _selection.clear();
    _selection.create_rect(x, y);
    _selection.update_selected_region(_image);
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
        _selection.set_last_rect_end(x, y);
    _selection.update_selected_region(_image);
}

void Canva::set_selection_end(int x, int y, int type) {
    apply_canva_drawing_factors(x, y);
    x = std::clamp(x, 0, _image.cols);
    y = std::clamp(y, 0, _image.rows);
    if (type == 2)
        _selection.add_point(cv::Point(x, y));
    else if (type == 3)
        return;
    else
        _selection.set_last_rect_end(x, y);
    if (!_selection.is_selection_active())
        _selection.clear();
    _selection.update_region_from_rects();
    _selection.update_selected_region(_image);
}

void Canva::normalize_selection() {
    _selection.normalize_selection();
    _selection.update_selected_region(_image);
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
    if (!_selection.is_selection_active()) {
        return;
    }
    if (_selection.get_active_handle() != Selection::ResizeHandle::None) {
        resize_selection_from_handle(x, y);
        return;
    }
    apply_canva_drawing_factors(x, y);
    int offset_x = -(_prev_x - x);
    int offset_y = -(_prev_y - y);
    _selection.move_selection_rects(offset_x, offset_y);
    // auto selected_region = _selection.get_selected_region();

    // selected_region.x += offset_x;
    // selected_region.y += offset_y;
    // _selection.set_start(cv::Point(selected_region.x, selected_region.y));
    // _selection.set_end(cv::Point(selected_region.x + selected_region.width,
    //                              selected_region.y + selected_region.height));
    _selection.update_selected_region(_image);
    _prev_x = x;
    _prev_y = y;
}

void Canva::stop_selection_grab() {
    _selection.set_active_handle(Selection::ResizeHandle::None);
    _selection.empty_unselected_mask();
}

void Canva::emplace_selection() {
    auto& layer            = _layers[_selected_layer].image;
    auto  selected_region  = _selection.get_selected_region();
    auto  selections_rects = _selection.get_selections_rects();

    if (layer.empty() || _selection.get_mask().empty() || selected_region.empty() ||
        selections_rects.empty()) {
        return;
    }

    // Iterer sur chaque rectangle de sélection
    for (const auto& rect : selections_rects) {
        // Calculez les coordonnées valides pour ce rectangle par rapport à la zone sélectionnée
        int x_start = std::max(0, rect.x);
        int y_start = std::max(0, rect.y);
        int x_end   = std::min(layer.cols, rect.x + rect.width);
        int y_end   = std::min(layer.rows, rect.y + rect.height);

        // Si les dimensions du rectangle sont invalides (retourne early)
        if (x_end <= x_start || y_end <= y_start) {
            continue;
        }

        // Créer un rectangle valide pour cette zone du calque
        cv::Rect valid_region(x_start, y_start, x_end - x_start, y_end - y_start);

        // Calculer la position du masque à appliquer
        int      mask_x_start = x_start - selected_region.x;
        int      mask_y_start = y_start - selected_region.y;
        cv::Rect mask_region(mask_x_start, mask_y_start, valid_region.width, valid_region.height);

        // Obtenez la zone correspondante du masque et copiez-la dans le calque
        cv::Mat selected_area = _selection.get_mask()(mask_region);
        selected_area.copyTo(layer(valid_region));
    }

    // Mettre à jour le calque
    mark_layer_for_update(_selected_layer);
}

} // namespace vipe