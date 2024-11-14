#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <gtkmm.h>
#include <iostream>
#include <vector>

#include "FloatingPanel.hpp"

namespace vipe {

struct Layer {
    cv::Mat image;
    bool    visible;
    int     opacity;
};

class Canva {
  public:
    Canva();
    ~Canva();
    void update();
    bool display_canva(const Cairo::RefPtr<Cairo::Context>& cr);
    void on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
    void set_image(const std::string& filename);
    void set_filename(const std::string& filename) { _current_filename = filename; }
    void set_point_pos(int x, int y);
    void set_color(cv::Scalar color) { _color = color; }
    void set_selected_layer(int index) { _selected_layer = index; }
    bool is_layer_visible(int index) { return _layers[index].visible; }
    void set_view_offset(int x, int y);
    void move_view(int x, int y, bool apply_zoom = true);
    void cursor_move_view(int x, int y);
    void set_zoom_factor(double factor);
    void zoom_view(double factor);
    void apply_canva_drawing_factors(int& x, int& y);
    void center_and_zoom_picture(int window_width, int window_height);
    void set_background_color(int r, int g, int b) { _background_color = cv::Scalar(b, g, r, 255); }
    Layer&                             get_selected_layer() { return _layers[_selected_layer]; }
    std::vector<Layer>&                get_layers() { return _layers; }
    std::string                        get_filename() { return _current_filename; }
    cv::Mat                            get_merged_image();
    void                               cursor_draw(int x1, int y1, int size);
    void                               cursor_erase(int x1, int y1, int size);
    void                               cursor_square(int x1, int y1, int size);
    void                               cursor_triangle(int x1, int y1, int size);
    void                               cursor_pastel(int x1, int y1, int size);
    void                               cursor_spray(int x1, int y1, int size);
    void                               color_fill(bool erase);
    cv::Scalar                         pick_color(int x, int y);
    void                               recalculate_background(const cv::Size& new_size);
    void                               convert_to_RGBA(const cv::Mat& src, cv::Mat& dst);
    Cairo::RefPtr<Cairo::ImageSurface> create_cairo_surface(const cv::Mat& image);
    Cairo::RefPtr<Cairo::Pattern>      create_repeating_pattern(const cv::Mat& image);
    void                               add_layer();
    void                               delete_layer(int index);
    void                               move_layer(int index, int new_index);
    void                               merge_layers();

  private:
    int                _view_offset_x;
    int                _view_offset_y;
    double             _zoom_factor;
    cv::Scalar         _background_color;
    cv::Mat            _background;
    cv::Mat            _bg_tiled;
    cv::Mat            _image;
    int                _selected_layer;
    std::vector<Layer> _layers;
    std::string        _current_filename;
    int                _prev_x;
    int                _prev_y;
    cv::Scalar         _color;
};
} // namespace vipe
