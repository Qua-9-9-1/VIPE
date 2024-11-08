#include "Canva.hpp"

namespace vipe {
    Canva::Canva(): _prev_x(-1), _prev_y(-1), _selected_layer(0), _background_color(98, 210, 98, 255)
    {
        _background = cv::imread("../assets/bg.png", cv::IMREAD_UNCHANGED);
        if (_background.empty()) {
            std::cerr << "Erreur lors du chargement de l'image de fond." << std::endl;
        }
        add_layer();
    }

    Canva::~Canva() {}

    bool Canva::display_canva(const Cairo::RefPtr<Cairo::Context>& cr)
    {
        try {
            cv::Mat image_rgba;
            cv::Mat bg_rgba;

            cr->set_source_rgba(
                _background_color[2] / 255.0,
                _background_color[1] / 255.0,
                _background_color[0] / 255.0,
                _background_color[3] / 255.0);
            cr->paint();
            convert_to_RGBA(_bg_tiled, bg_rgba);
            auto bg_surface = create_cairo_surface(bg_rgba);
            cr->set_source(bg_surface, _view_offset_x, _view_offset_y);
            cr->paint();
            for (const auto& layer : _layers) {
                if (!layer.visible) continue;
                convert_to_RGBA(layer.image, image_rgba);
                auto surface = create_cairo_surface(image_rgba);
                cr->set_source(surface, _view_offset_x, _view_offset_y);
                cr->paint_with_alpha(layer.opacity / 100.0);
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return false;
        }
    }

    void Canva::recalculate_background(const cv::Size& new_size)
    {
        cv::Mat bg_resized;
        cv::resize(_background, bg_resized, cv::Size(), 10.0, 10.0, cv::INTER_NEAREST);

        _bg_tiled = cv::Mat(new_size, bg_resized.type());
        for (int y = 0; y < _bg_tiled.rows; y += bg_resized.rows) {
            for (int x = 0; x < _bg_tiled.cols; x += bg_resized.cols) {
                int width = std::min(bg_resized.cols, _bg_tiled.cols - x);
                int height = std::min(bg_resized.rows, _bg_tiled.rows - y);
                cv::Rect roi(0, 0, width, height);
                bg_resized(roi).copyTo(_bg_tiled(cv::Rect(x, y, width, height)));
            }
        }
    }

    void Canva::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
    {
    }

    void Canva::set_image(const std::string& filename)
    {
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

    void Canva::convert_to_RGBA(const cv::Mat& src, cv::Mat& dst)
    {
        if (src.channels() == 3)
            cv::cvtColor(src, dst, cv::COLOR_BGR2BGRA);
        else if (src.channels() == 4)
            dst = src.clone();
        // else
        //     throw std::runtime_error("Image non supportée.");
    }

    Cairo::RefPtr<Cairo::ImageSurface> Canva::create_cairo_surface(const cv::Mat& image)
    {
        int stride = Cairo::ImageSurface::format_stride_for_width(
            Cairo::FORMAT_ARGB32, image.cols);
        auto surface = Cairo::ImageSurface::create(image.data,
            Cairo::FORMAT_ARGB32, image.cols, image.rows, stride);

        if (surface->get_status() != CAIRO_STATUS_SUCCESS)
            throw std::runtime_error("Erreur lors de la création de la surface.");
        return surface;
    }

    cv::Mat Canva::get_merged_image()
    {
        merge_layers();
        return _image;
    }

    void Canva::add_layer()
    {
        Layer layer;
        layer.image = cv::Mat(_image.size(), _image.type(), cv::Scalar(0, 0, 0, 0));
        layer.visible = true;
        layer.opacity = 100;
        _selected_layer = _layers.size();
        _layers.push_back(layer);
    }

    void Canva::delete_layer(int index)
    {
        if (index < 0 || index >= _layers.size()) {
            throw std::out_of_range("Couche hors limites");
        }
        _layers.erase(_layers.begin() + index);
    }

    void Canva::move_layer(int index, int new_index)
    {
        if (index < 0 || index >= _layers.size()
        || new_index < 0 || new_index >= _layers.size()) {
            throw std::out_of_range("Couche hors limites");
        }
        auto layer_to_move = _layers[index];
        _layers.erase(_layers.begin() + index);
        _layers.insert(_layers.begin() + new_index, layer_to_move);
    }

    void Canva::merge_layers()
    {
        if (_layers.empty()) return;
        cv::Mat merged = cv::Mat::zeros(_image.size(), _image.type());
        cv::Mat image;

        for (auto layer : _layers) {
            if (layer.image.empty() || !layer.visible) continue;
            cv::resize(layer.image, image, merged.size(), 0, 0, cv::INTER_NEAREST);
            cv::addWeighted(merged, 1.0, image, 1.0, 0.0, merged);
        }
        if (merged.empty())
            merged = cv::Mat(_image.size(), _image.type(), cv::Scalar(0, 0, 0, 0));
        _image = merged;
    }
}