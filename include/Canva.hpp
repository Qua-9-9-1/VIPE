#pragma once

#include <opencv2/opencv.hpp>
#include <gtk/gtk.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <gtkmm.h>
#include <iostream>
#include <vector>
#include <gtkmm/clipboard.h>
#include <opencv2/imgcodecs.hpp>

#include "Selection.hpp"
#include "FloatingPanel.hpp"

namespace vipe {

struct Layer {
    cv::Mat     image;
    bool        visible;
    int         opacity;
    std::string name;
    int         blend_mode;
};

struct CachedLayer {
    cv::Mat cached_image;
    bool    visible;
    int     blend_mode;
    double  opacity;
    double  zoom_factor;
    bool    needs_update;
};

class Canva {
  public:
    Canva();
    ~Canva();
    void update();
    bool display_canva(const Cairo::RefPtr<Cairo::Context>& cr);
    void draw_selection_rect(const Cairo::RefPtr<Cairo::Context>& cr);
    void render_selection(cv::Mat& canvas);
    void update_layer_cache();
    void create_blank_picture(int width, int height);
    void set_image(const std::string& filename);
    void set_filename(const std::string& filename) { _current_filename = filename; }
    void set_point_pos(int x, int y);
    void set_color(cv::Scalar color) { _color = color; }
    void set_selected_layer(int index);
    bool is_layer_visible(int index) { return _layers[index].visible; }
    void set_view_offset(int x, int y);
    void move_view(int x, int y, bool apply_zoom = true);
    void cursor_move_view(int x, int y);
    void set_zoom_factor(double factor);
    void zoom_view(double factor);
    void apply_canva_drawing_factors(int& x, int& y);
    void center_and_zoom_picture(int window_width, int window_height);
    void update_background();
    void set_background_color(int r, int g, int b) { _background_color = cv::Scalar(b, g, r, 255); }
    Layer&              get_selected_layer() { return _layers[_selected_layer]; }
    int                 get_selected_layer_index() { return _selected_layer; }
    std::vector<Layer>& get_layers() { return _layers; }
    std::string         get_filename() { return _current_filename; }
    cv::Mat             get_merged_image();
    void                copy_from_layer_to_selection();
    void                empty_selection_on_layer();

    void       cursor_draw(int x1, int y1, int size, bool smooth = true);
    void       cursor_erase(int x1, int y1, int size, bool smooth = true);
    void       cursor_square(int x1, int y1, int size, bool smooth = true);
    void       cursor_triangle(int x1, int y1, int size, bool smooth = true);
    void       cursor_pastel(int x1, int y1, int size, bool smooth = true);
    void       cursor_spray(int x1, int y1, int size, bool smooth = true);
    void       line_draw(int x, int y, int size, bool smooth = true);
    void       flood_fill(int x, int y, double tolerance);
    cv::Scalar pick_color(int x, int y);
    void       empty_unselected_mask();

    bool init_resize_selection(int x, int y);
    void resize_selection_from_handle(int x, int y);
    void set_selection_start(int x, int y, int type, bool multiple = false);
    void resize_selection(int x, int y, int type);
    void set_selection_end(int x, int y, int type);
    void normalize_selection();
    void init_move_selection(int x, int y);
    void move_selection(int x, int y);
    void emplace_selection();
    void stop_selection_grab();
    bool selection_out_of_bounds();
    void copy_selection_to_clipboard();
    void cut_selection_to_clipboard();
    void paste_from_clipboard();

    void                               recalculate_background(const cv::Size& new_size);
    void                               convert_to_RGBA(const cv::Mat& src, cv::Mat& dst);
    Cairo::RefPtr<Cairo::ImageSurface> create_cairo_surface(const cv::Mat& image);
    Cairo::RefPtr<Cairo::Pattern>      create_repeating_pattern(const cv::Mat& image);
    void                               add_layer();
    void                               add_layer_from_image(const std::string& filename);
    void                               delete_layer(int index);
    void                               move_layer(int index, int new_index);
    void                               move_layer_up(int index);
    void                               move_layer_down(int index);
    void                               merge_layers();
    void                               mark_layer_for_update(int index);
    cv::Mat blend_normal(const cv::Mat& base, const cv::Mat& overlay, double alpha);

  private:
    int                           _view_offset_x;
    int                           _view_offset_y;
    double                        _zoom_factor;
    cv::Scalar                    _background_color;
    cv::Mat                       _background;
    cv::Mat                       _bg_tiled;
    cv::Mat                       _image;
    int                           _selected_layer;
    std::vector<Layer>            _layers;
    std::string                   _current_filename;
    int                           _prev_x;
    int                           _prev_y;
    cv::Scalar                    _color;
    Cairo::RefPtr<Cairo::Pattern> _bg_pattern;
    std::vector<CachedLayer>      _cached_layers;
    Selection                     _selection;
};
} // namespace vipe
