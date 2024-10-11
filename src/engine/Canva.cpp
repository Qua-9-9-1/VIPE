#include "Canva.hpp"

namespace vipe {
    Canva::Canva()
    {
        _prev_x = -1;
        _prev_y = -1;
        _background = cv::imread("../assets/bg.png", cv::IMREAD_UNCHANGED);
        if (_background.empty()) {
            std::cerr << "Erreur lors du chargement de l'image de fond." << std::endl;
        }
    }

    Canva::~Canva() {}

    bool Canva::display_canva(const Cairo::RefPtr<Cairo::Context>& cr)
    {
        try {
            cv::Mat image_rgba;
            cv::Mat bg_rgba;

            convert_to_RGBA(_image, image_rgba);
            auto surface = create_cairo_surface(image_rgba);
            convert_to_RGBA(_bg_tiled, bg_rgba);
            auto bg_surface = create_cairo_surface(bg_rgba);
            cr->set_source(bg_surface, 0, 0);
            cr->paint();
            cr->set_source(surface, 0, 0);
            cr->paint_with_alpha(1.0);
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
                // Calculer la région d'intérêt en tenant compte des limites
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
        _image = cv::imread(filename, cv::IMREAD_UNCHANGED);
        if (_image.empty()) {
            std::cerr << "Erreur lors du chargement de l'image." << std::endl;
        }
        convert_to_RGBA(_image, _image);
        recalculate_background(cv::Size(_image.cols, _image.rows));
    }

    void Canva::draw_line(int x1, int y1)
    {
        if (_image.empty()) {
            return;
        }
        if (_image.channels() == 4 && (_prev_x != -1 && _prev_y != -1)) {
            cv::line(_image, cv::Point(_prev_x, _prev_y), cv::Point(x1, y1), _color, 2);
        }
        set_point_pos(x1, y1);
    }

    void Canva::erase(int x1, int y1)
    {
        if (_image.empty()) {
            return;
        }
        if (_image.channels() == 4 && (_prev_x != -1 && _prev_y != -1)) {
            cv::line(_image, cv::Point(_prev_x, _prev_y), cv::Point(x1, y1), cv::Scalar(0, 0, 0, 0), 2);
        }
        set_point_pos(x1, y1);
    }

    void Canva::convert_to_RGBA(const cv::Mat& src, cv::Mat& dst)
    {
        if (src.channels() == 3)
            cv::cvtColor(src, dst, cv::COLOR_BGR2BGRA);
        else if (src.channels() == 4)
            dst = src.clone();
        else
            throw std::runtime_error("Format d'image non supporté.");
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

}