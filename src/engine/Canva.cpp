#include "Canva.hpp"

namespace vipe {
Canva::Canva()
    : _view_offset_x(0), _view_offset_y(0), _zoom_factor(1.0), _prev_x(-1), _prev_y(-1),
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
        cv::Mat image_rgba;
        cv::Mat bg_rgba;

        convert_to_RGBA(_bg_tiled, bg_rgba);
        auto bg_pattern = create_repeating_pattern(bg_rgba);
        cr->set_source(bg_pattern);
        cr->rectangle(_view_offset_x, _view_offset_y, _image.cols * _zoom_factor,
                      _image.rows * _zoom_factor);
        cr->fill();
        for (const auto& layer : _layers) {
            if (!layer.visible)
                continue;
            cr->save();
            convert_to_RGBA(layer.image, image_rgba);
            cv::resize(image_rgba, image_rgba, cv::Size(), _zoom_factor, _zoom_factor,
                       cv::INTER_NEAREST);
            auto surface = create_cairo_surface(image_rgba);
            cr->set_source(surface, _view_offset_x, _view_offset_y);
            cr->scale(_zoom_factor, _zoom_factor);
            cr->paint_with_alpha(layer.opacity / 100.0);
            cr->restore();
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

void Canva::create_blank_picture(int width, int height) {
    _image = cv::Mat(width, height, CV_8UC4, cv::Scalar(255, 255, 255, 255));
    recalculate_background(cv::Size(_image.cols, _image.rows));
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

void Canva::set_image(const std::string& filename) {
    _layers.clear();
    add_layer();
    _image = cv::imread(filename, cv::IMREAD_UNCHANGED);
    if (_image.empty()) {
        std::cerr << "Erreur lors du chargement de l'image." << std::endl;
    }
    convert_to_RGBA(_image, _image);
    recalculate_background(cv::Size(_image.cols, _image.rows));
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
    // Créer une surface Cairo à partir de l'image
    int  stride  = Cairo::ImageSurface::format_stride_for_width(Cairo::FORMAT_ARGB32, image.cols);
    auto surface = Cairo::ImageSurface::create(image.data, Cairo::FORMAT_ARGB32, image.cols,
                                               image.rows, stride);

    if (surface->get_status() != CAIRO_STATUS_SUCCESS) {
        throw std::runtime_error("Erreur lors de la création de la surface.");
    }

    // Créer un motif répétitif à partir de cette surface
    auto pattern = Cairo::SurfacePattern::create(surface);
    pattern->set_extend(Cairo::Extend::EXTEND_REPEAT);

    return pattern;
}

cv::Mat Canva::get_merged_image() {
    merge_layers();
    return _image;
}

void Canva::add_layer() {
    Layer layer;
    layer.image     = cv::Mat(_image.size(), _image.type(), cv::Scalar(0, 0, 0, 0));
    layer.visible   = true;
    layer.opacity   = 100;
    _selected_layer = _layers.size();
    _layers.push_back(layer);
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
}

void Canva::merge_layers() {
    if (_layers.empty())
        return;
    cv::Mat merged = cv::Mat::zeros(_image.size(), _image.type());
    cv::Mat image;

    for (auto layer : _layers) {
        if (layer.image.empty() || !layer.visible)
            continue;
        cv::resize(layer.image, image, merged.size(), 0, 0, cv::INTER_NEAREST);
        cv::addWeighted(merged, 1.0, image, 1.0, 0.0, merged);
    }
    if (merged.empty())
        merged = cv::Mat(_image.size(), _image.type(), cv::Scalar(0, 0, 0, 0));
    _image = merged;
}
} // namespace vipe