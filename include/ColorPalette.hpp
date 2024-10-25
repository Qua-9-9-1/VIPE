#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <gtkmm.h>
#include <iostream>
#include <vector>

#include "FloatingPanel.hpp"

namespace vipe {
    class ColorPalette {
        public:
            ColorPalette();
            ~ColorPalette();
            cv::Scalar get_first_color() { return _first_color; }
            cv::Scalar get_second_color() { return _second_color; }
            void set_first_color(cv::Scalar color) { _first_color = color; }
            void set_second_color(cv::Scalar color) { _second_color = color; }
            FloatingPanel& get_color_palette() { return _color_palette; }
        private:
            cv::Scalar      _first_color;
            cv::Scalar      _second_color;
            FloatingPanel   _color_palette;
            Gtk::Box        _color_box;
    };
}