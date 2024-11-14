#pragma once

#include <gtkmm.h>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

#include "FloatingPanel.hpp"

namespace vipe {
class ColorPalette {
  public:
    ColorPalette();
    ~ColorPalette();
    void           build_selected_colors();
    void           build_chromatic_circle();
    bool           on_draw_chromatic_circle(const Cairo::RefPtr<Cairo::Context>& cr);
    void           hsv_to_rgb(double h, double s, double v, double& r, double& g, double& b);
    void           rgb_to_hsv(double r, double g, double b, double& h, double& s, double& v);
    void           build_colors_grid();
    void           build_hiddeable_box();
    void           build_colors_gauges();
    void           update_gauges_from_color();
    void           update_color_from_gauges();
    cv::Scalar     get_first_color() { return _first_color; }
    cv::Scalar     get_second_color() { return _second_color; }
    double         get_hue(cv::Scalar color);
    double         get_saturation(cv::Scalar color);
    double         get_value(cv::Scalar color);
    bool           update_cursor(const Cairo::RefPtr<Cairo::Context>& cr);
    Gdk::RGBA      get_gtk_color(cv::Scalar color);
    cv::Scalar     get_cv_color(Gdk::RGBA color);
    void           set_first_color(cv::Scalar color);
    void           set_hsv_first_color(double h, double s, double v);
    void           set_second_color(cv::Scalar color);
    bool           on_chromatic_circle_click(GdkEventButton* event);
    bool           on_chromatic_circle_release(GdkEventButton* event);
    bool           on_chromatic_circle_motion(GdkEventMotion* event);
    bool           update_color_from_position(int x, int y);
    void           switch_colors();
    FloatingPanel& get_color_palette() { return _color_palette; }

  private:
    int               _cursor_x = -1;
    int               _cursor_y = -1;
    Gtk::DrawingArea* _cursor_area;
    bool              _is_dragging;
    bool              _no_gauge_signal;
    cv::Scalar        _first_color;
    cv::Scalar        _second_color;
    FloatingPanel     _color_palette;
    Gtk::Box          _color_box;
    Gtk::Box          _selected_colors_box;
    Gtk::Box          _chromatic_circle_box;
    Gtk::Box          _colors_grid_box;
    Gtk::Box          _hiddeable_box;
    Gtk::EventBox*    _first_color_box;
    Gtk::EventBox*    _second_color_box;
    Gtk::DrawingArea* _chromatic_circle;
    Gtk::Scale*       _red_gauge        = nullptr;
    Gtk::Scale*       _green_gauge      = nullptr;
    Gtk::Scale*       _blue_gauge       = nullptr;
    Gtk::Scale*       _hue_gauge        = nullptr;
    Gtk::Scale*       _saturation_gauge = nullptr;
    Gtk::Scale*       _value_gauge      = nullptr;
    Gtk::Scale*       _alpha_gauge      = nullptr;
};
} // namespace vipe