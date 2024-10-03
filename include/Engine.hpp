#pragma once

#include <gtkmm.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <memory>
#include "Menu.hpp"

namespace vipe {
    class Menu;

    class MyWindow : public Gtk::Window {
        public:
            MyWindow();
            ~MyWindow();
            Gtk::VBox& get_vbox() { return _vbox; }
            Gtk::DrawingArea& get_drawing_area() { return _drawing_area; }
            cv::Mat& get_current_image() { return _current_image; }
            std::string& get_current_filename() { return _current_filename; }
            bool onDraw(const Cairo::RefPtr<Cairo::Context>& cr);
            // file options
            void create_file_option();
            void create_display_option();
            void create_image_option();
            void create_effect_option();
            void new_file();
            void open_file();
            void save_file();
            void save_as_file();
            void close_file();
            // display options
            void fullscreen();
            // image options
            void resize_image();
            // effect options
            void blur_effect();
        private:
            Menu                    _menu;
            Gtk::VBox               _vbox;
            Gtk::DrawingArea        _drawing_area;
            cv::Mat                 _current_image;
            std::string             _current_filename;
    };
}