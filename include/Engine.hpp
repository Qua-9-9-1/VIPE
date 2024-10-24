#pragma once

#include <gtkmm.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <memory>
#include <vector>

#include "Menu.hpp"
#include "Canva.hpp"
#include "Toolkit.hpp"

namespace vipe {
    class Menu;
    class Canva;
    class Toolkit;

    class MyWindow : public Gtk::Window {
        public:
            MyWindow();
            ~MyWindow();
            void build_menu();
            void build_drawing_area();
            void build_panels();
            Gtk::VBox& get_vbox() { return _vbox; }
            Gtk::DrawingArea& get_drawing_area() { return _drawing_area; }
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
            //events
            bool on_button_press(GdkEventButton* event);
            bool on_button_release(GdkEventButton* event);
            bool on_motion_notify(GdkEventMotion* event);
            bool on_key_press(GdkEventKey* event);
            bool file_shortcuts(GdkEventKey* event);
        private:
            Menu                    _menu;
            Gtk::VBox               _vbox;
            Gtk::DrawingArea        _drawing_area;
            std::shared_ptr<Canva> _canva;
            std::vector<Canva>     _canvas;
            Gtk::Overlay           _overlay;
            Gtk::Fixed             _fixed_layout;
            Toolkit                _toolkit;
            
    };
}