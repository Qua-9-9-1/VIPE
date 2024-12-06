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
        for (const auto& cached_layer : _cached_layers) {
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
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

void Canva::update_layer_cache() {
    if (_cached_layers.size() != _layers.size()) {
        _cached_layers.resize(_layers.size());
    }

    for (size_t i = 0; i < _layers.size(); ++i) {
        const auto& layer        = _layers[i];
        auto&       cached_layer = _cached_layers[i];

        if (layer.visible != cached_layer.visible || layer.blend_mode != cached_layer.blend_mode ||
            layer.opacity != cached_layer.opacity || _zoom_factor != cached_layer.zoom_factor ||
            cached_layer.needs_update) {
            convert_to_RGBA(layer.image, cached_layer.cached_image);
            cv::resize(cached_layer.cached_image, cached_layer.cached_image, cv::Size(),
                       _zoom_factor, _zoom_factor, cv::INTER_NEAREST);
            cached_layer.visible      = layer.visible;
            cached_layer.blend_mode   = layer.blend_mode;
            cached_layer.opacity      = layer.opacity;
            cached_layer.zoom_factor  = _zoom_factor;
            cached_layer.needs_update = false;
        }
    }
}

void Canva::create_blank_picture(int width, int height) {
    _image = cv::Mat(width, height, CV_8UC4, cv::Scalar(255, 255, 255, 255));
    update_background();
    _layers[0].image = _image;
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

void Canva::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {}

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

void Canva::add_layer() {
    Layer layer;
    layer.image      = cv::Mat(_image.size(), _image.type(), cv::Scalar(0, 0, 0, 0));
    layer.visible    = true;
    layer.opacity    = 100;
    layer.name       = "Calque " + std::to_string(_layers.size() + 1);
    layer.blend_mode = 0;
    _selected_layer  = _layers.size();
    _layers.push_back(layer);
}

void Canva::add_layer_from_image(const std::string& filename) {
    Layer layer;
    layer.image = cv::imread(filename, cv::IMREAD_UNCHANGED);
    if (layer.image.empty()) {
        std::cerr << "Erreur lors du chargement de l'image." << std::endl;
        return;
    }
    convert_to_RGBA(layer.image, layer.image);
    int new_width  = std::max(layer.image.cols, _image.cols);
    int new_height = std::max(layer.image.rows, _image.rows);

    if (new_width > _image.cols || new_height > _image.rows) {
        cv::Mat new_canva(cv::Size(new_width, new_height), _image.type(), cv::Scalar(0, 0, 0, 0));
        _image.copyTo(new_canva(cv::Rect(0, 0, _image.cols, _image.rows)));
        _image = new_canva;
        for (auto& existing_layer : _layers) {
            cv::Mat resized_layer(cv::Size(new_width, new_height), existing_layer.image.type(),
                                  cv::Scalar(0, 0, 0, 0));
            existing_layer.image.copyTo(resized_layer(
                cv::Rect(0, 0, existing_layer.image.cols, existing_layer.image.rows)));
            existing_layer.image = resized_layer;
        }
    }
    layer.name       = "Calque " + std::to_string(_layers.size() + 1);
    layer.visible    = true;
    layer.opacity    = 100;
    layer.blend_mode = 0;
    _selected_layer  = _layers.size();
    _layers.push_back(layer);
    update_background();
}

void Canva::delete_layer(int index) {
    if (index < 0 || index >= _layers.size()) {
        throw std::out_of_range("Couche hors limites");
    }
    _layers.erase(_layers.begin() + index);
}

void Canva::move_layer(int index, int new_index) {
    if (index < 0 || index >= _layers.size() || new_index < 0 || new_index >= _layers.size()) {
        throw std::out_of_range("Couche hors limites");
    }
    auto layer_to_move = _layers[index];
    _layers.erase(_layers.begin() + index);
    _layers.insert(_layers.begin() + new_index, layer_to_move);
    _selected_layer = new_index;
}

void Canva::move_layer_up(int index) {
    if (index < 0 || index >= _layers.size()) {
        throw std::out_of_range("Couche hors limites");
    }
    if (index == 0)
        return;
    auto layer_to_move = _layers[index];
    _layers.erase(_layers.begin() + index);
    _layers.insert(_layers.begin() + index - 1, layer_to_move);
    _selected_layer = index - 1;
}

void Canva::move_layer_down(int index) {
    if (index < 0 || index >= _layers.size()) {
        throw std::out_of_range("Couche hors limites");
    }
    if (index == _layers.size() - 1)
        return;
    auto layer_to_move = _layers[index];
    _layers.erase(_layers.begin() + index);
    _layers.insert(_layers.begin() + index + 1, layer_to_move);
    _selected_layer = index + 1;
}

void Canva::merge_layers() { _image = get_merged_image(); }

cv::Mat Canva::blend_normal(const cv::Mat& base, const cv::Mat& overlay, double alpha) {
    CV_Assert(base.type() == CV_8UC4 && overlay.type() == CV_8UC4);
    cv::Mat result = base.clone();
    for (int y = 0; y < base.rows; ++y) {
        for (int x = 0; x < base.cols; ++x) {
            cv::Vec4b bg_pixel = base.at<cv::Vec4b>(y, x);
            cv::Vec4b fg_pixel = overlay.at<cv::Vec4b>(y, x);

            double fg_alpha = (fg_pixel[3] / 255.0) * alpha;
            double bg_alpha = bg_pixel[3] / 255.0;

            double out_alpha = fg_alpha + bg_alpha * (1.0 - fg_alpha);

            if (out_alpha > 0) {
                for (int c = 0; c < 3; ++c) {
                    result.at<cv::Vec4b>(y, x)[c] = static_cast<uchar>(
                        (fg_pixel[c] * fg_alpha + bg_pixel[c] * bg_alpha * (1.0 - fg_alpha)) /
                        out_alpha);
                }
                result.at<cv::Vec4b>(y, x)[3] = static_cast<uchar>(out_alpha * 255);
            }
        }
    }
    return result;
}

cv::Mat blend_multiply(const cv::Mat& base, const cv::Mat& overlay) {
    CV_Assert(base.type() == CV_8UC4 && overlay.type() == CV_8UC4);
    cv::Mat result = base.clone();
    for (int y = 0; y < base.rows; ++y) {
        for (int x = 0; x < base.cols; ++x) {
            cv::Vec4b bg_pixel = base.at<cv::Vec4b>(y, x);
            cv::Vec4b fg_pixel = overlay.at<cv::Vec4b>(y, x);

            for (int c = 0; c < 3; ++c) {
                result.at<cv::Vec4b>(y, x)[c] =
                    static_cast<uchar>((bg_pixel[c] / 255.0) * (fg_pixel[c] / 255.0) * 255);
            }
            result.at<cv::Vec4b>(y, x)[3] = bg_pixel[3];
        }
    }
    return result;
}

void Canva::mark_layer_for_update(int layer_index) {
    if (layer_index < _cached_layers.size()) {
        _cached_layers[layer_index].needs_update = true;
    }
}

} // namespace vipe