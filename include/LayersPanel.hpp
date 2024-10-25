#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <gtkmm.h>
#include <iostream>
#include <vector>

#include "Canva.hpp"

namespace vipe {
    class LayersPanel {
        public:
            LayersPanel(Canva& canva);
            ~LayersPanel();
            Gtk::Box& get_layer_panel() { return _layer_panel; }

            void build_layer_panel();
            void update_layer_list();
            void clear_layer_list();
            void add_layer_to_list(int i);
            Gtk::EventBox* create_event_box(int i, Gtk::Box* layer_box);
            void add_delete_button(Gtk::Box* layer_box, int i);
            void add_visibility_button(Gtk::Box* layer_box, int i);
            void add_add_layer_button();

            void on_layer_clicked(int index);
            bool on_layer_drag_motion(int x, int y);
            void on_add_layer();
            void on_delete_layer(int index);
            void toggle_layer_visibility(int index);
        private:
            Canva&              _canva;
            Gtk::Box            _layer_list_container;
            FloatingPanel       _layer_panel;
    };
}