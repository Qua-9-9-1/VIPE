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
        bool visible;
        int opacity;
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
            void set_point_pos(int x, int y) { _prev_x = x; _prev_y = y; }
            void set_color(cv::Scalar color) { _color = color; }
            void set_selected_layer(int index) { _selected_layer = index; }
            bool is_layer_visible(int index) { return _layers[index].visible; }
            void move_view(int x, int y) { _view_offset_x += x; _view_offset_y += y; }
            Layer& get_selected_layer() { return _layers[_selected_layer]; }
            std::vector<Layer>& get_layers() { return _layers; }
            std::string get_filename() { return _current_filename; }
            cv::Mat get_merged_image();
            void draw_line(int x1, int y1, int size);
            void erase(int x1, int y1, int size);
            void recalculate_background(const cv::Size& new_size);
            void convert_to_RGBA(const cv::Mat& src, cv::Mat& dst);
            Cairo::RefPtr<Cairo::ImageSurface> create_cairo_surface(const cv::Mat& image);
            void add_layer();
            void delete_layer(int index);
            void move_layer(int index, int new_index);
            void merge_layers();
        private:
            int                  _view_offset_x = 0;
            int                  _view_offset_y = 0;
            std::string          _background_color;
            cv::Mat              _background;
            cv::Mat              _bg_tiled;
            cv::Mat              _image;
            int                  _selected_layer;
            std::vector<Layer>   _layers;
            std::string          _current_filename;
            int                  _prev_x;
            int                  _prev_y;
            cv::Scalar           _color;
    };
}
