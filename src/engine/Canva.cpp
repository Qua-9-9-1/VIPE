#include "Canva.hpp"

namespace vipe {
Canva::Canva()
    : _view_offset_x(0), _view_offset_y(0), _zoom_factor(0.1), _prev_x(-1), _prev_y(-1),
      _selected_layer(0), _background_color(255, 235, 225, 255) {
    _background = cv::imread("../assets/bg.png", cv::IMREAD_UNCHANGED);
    if (_background.empty()) {
        std::cerr << "Erreur lors du chargement de l'image de fond." << std::endl;
    }
    add_layer();
}

Canva::~Canva() {}

bool Canva::display_canva(const Cairo::RefPtr<Cairo::Context>& cr) {
    cr->set_source_rgba(_background_color[2] / 255.0, _background_color[1] / 255.0,
                        _background_color[0] / 255.0, _background_color[3] / 255.0);
    cr->paint();
    if (_image.empty())
        return false;
    try {
        cv::Mat bg_rgba;
        convert_to_RGBA(_bg_tiled, bg_rgba);
        auto bg_pattern = create_repeating_pattern(bg_rgba);
        cr->set_source(bg_pattern);
        cr->rectangle(_view_offset_x, _view_offset_y, _image.cols * _zoom_factor,
                      _image.rows * _zoom_factor);
        cr->fill();
        update_layer_cache();
        for (size_t i = 0; i < _cached_layers.size(); ++i) {
            const auto& cached_layer = _cached_layers[i];

            if (!cached_layer.visible)
                continue;
            cr->save();
            auto surface = create_cairo_surface(cached_layer.cached_image);
            cr->set_source(surface, _view_offset_x, _view_offset_y);
            switch (cached_layer.blend_mode) {
            case 1:
                cr->set_operator(Cairo::OPERATOR_ADD);
                break;
            default:
                cr->set_operator(Cairo::OPERATOR_OVER);
                break;
            }
            cr->paint_with_alpha(cached_layer.opacity / 100.0);
            cr->restore();
            if (i == _selected_layer) {
                cv::Mat layer_with_selection = cached_layer.cached_image.clone();

                // Ajouter la sélection temporaire si active
                render_selection(layer_with_selection);

                // Dessiner la couche modifiée (avec la sélection)
                auto selection_surface = create_cairo_surface(layer_with_selection);
                cr->set_source(selection_surface, _view_offset_x, _view_offset_y);
                cr->paint_with_alpha(cached_layer.opacity / 100.0);
            }
        }
        draw_selection_rect(cr);
        return true;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
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
    }
}

void Canva::create_blank_picture(int width, int height) {
    _image = cv::Mat(width, height, CV_8UC4, cv::Scalar(255, 255, 255, 255));
    update_background();
    _layers[0].image = _image;
    update_selected_region();
}

void Canva::recalculate_background(const cv::Size& new_size) {
    cv::Mat bg_resized;
    cv::resize(_background, bg_resized, cv::Size(), 10.0, 10.0, cv::INTER_NEAREST);

    _bg_tiled = cv::Mat(new_size, bg_resized.type());
    for (int y = 0; y < _bg_tiled.rows; y += bg_resized.rows) {
        for (int x = 0; x < _bg_tiled.cols; x += bg_resized.cols) {
            int      width  = std::min(bg_resized.cols, _bg_tiled.cols - x);
            int      height = std::min(bg_resized.rows, _bg_tiled.rows - y);
            cv::Rect roi(0, 0, width, height);
            bg_resized(roi).copyTo(_bg_tiled(cv::Rect(x, y, width, height)));
        }
    }
}

void Canva::set_selected_layer(int index) {
    if (index < 0 || index >= _layers.size()) {
        throw std::out_of_range("Couche hors limites");
    }
    _selected_layer = index;
    update_selected_region();
}

void Canva::update_background() {
    recalculate_background(cv::Size(_image.cols, _image.rows));
    convert_to_RGBA(_bg_tiled, _bg_tiled);
    _bg_pattern = create_repeating_pattern(_bg_tiled);
}

void Canva::set_image(const std::string& filename) {
    _layers.clear();
    add_layer();
    _image = cv::imread(filename, cv::IMREAD_UNCHANGED);
    if (_image.empty()) {
        std::cerr << "Erreur lors du chargement de l'image." << std::endl;
    }
    convert_to_RGBA(_image, _image);
    update_background();
    _layers[0].image = _image;
    update_selected_region();
}

void Canva::convert_to_RGBA(const cv::Mat& src, cv::Mat& dst) {
    if (src.channels() == 3)
        cv::cvtColor(src, dst, cv::COLOR_BGR2BGRA);
    else if (src.channels() == 4)
        dst = src.clone();
    // else
    //     throw std::runtime_error("Image non supportée.");
}

Cairo::RefPtr<Cairo::ImageSurface> Canva::create_cairo_surface(const cv::Mat& image) {
    int  stride  = Cairo::ImageSurface::format_stride_for_width(Cairo::FORMAT_ARGB32, image.cols);
    auto surface = Cairo::ImageSurface::create(image.data, Cairo::FORMAT_ARGB32, image.cols,
                                               image.rows, stride);

    if (surface->get_status() != CAIRO_STATUS_SUCCESS)
        throw std::runtime_error("Erreur lors de la création de la surface.");
    return surface;
}

Cairo::RefPtr<Cairo::Pattern> Canva::create_repeating_pattern(const cv::Mat& image) {
    int  stride  = Cairo::ImageSurface::format_stride_for_width(Cairo::FORMAT_ARGB32, image.cols);
    auto surface = Cairo::ImageSurface::create(image.data, Cairo::FORMAT_ARGB32, image.cols,
                                               image.rows, stride);

    if (surface->get_status() != CAIRO_STATUS_SUCCESS) {
        throw std::runtime_error("Erreur lors de la création de la surface.");
    }
    auto pattern = Cairo::SurfacePattern::create(surface);
    pattern->set_extend(Cairo::Extend::EXTEND_REPEAT);

    return pattern;
}

cv::Mat Canva::get_merged_image() {
    if (_layers.empty())
        return cv::Mat(_image.size(), _image.type(), cv::Scalar(0, 0, 0, 0));
    cv::Mat merged = cv::Mat::zeros(_image.size(), _image.type());
    cv::Mat image;

    for (auto layer : _layers) {
        if (layer.image.empty() || !layer.visible || layer.opacity == 0)
            continue;
        cv::resize(layer.image, image, merged.size(), 0, 0, cv::INTER_NEAREST);
        if (layer.blend_mode == 0)
            merged = blend_normal(merged, layer.image, layer.opacity / 100.0);
        if (layer.blend_mode == 1)
            cv::addWeighted(merged, 1.0, image, 1.0, 0.0, merged);
        if (layer.blend_mode == 2)
            cv::subtract(merged, image, merged);
        if (layer.blend_mode == 3)
            cv::addWeighted(merged, 1.0, image, 1.0, 0.0, merged);
    }
    if (merged.empty())
        merged = cv::Mat(_image.size(), _image.type(), cv::Scalar(0, 0, 0, 0));
    return merged;
}

void Canva::update_selected_region() {
    if (_selection.is_selection_active()) {
        auto selection_start = _selection.get_start();
        auto selection_end   = _selection.get_end();
        _selected_region     = cv::Rect(std::min(selection_start.x, selection_end.x),
                                        std::min(selection_start.y, selection_end.y),
                                        std::abs(selection_end.x - selection_start.x),
                                        std::abs(selection_end.y - selection_start.y));
        auto& layer          = _layers[_selected_layer].image;
        if (layer.empty()) {
            return;
        }
        cv::Mat region_content = layer(_selected_region).clone();
        _selection.set_mask(region_content);
    } else {
        _selected_region = cv::Rect(0, 0, _image.cols, _image.rows);
    }
}

bool Canva::selection_out_of_bounds() {
    return _selected_region.x < 0 || _selected_region.y < 0 ||
           _selected_region.x + _selected_region.width > _image.cols ||
           _selected_region.y + _selected_region.height > _image.rows;
}

} // namespace vipe