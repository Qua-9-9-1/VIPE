#pragma once

#include <gtkmm.h>
#include <iostream>

namespace vipe {

    class MyWindow;

    class FloatingPanel : public Gtk::Box {
        public:
            FloatingPanel();
            ~FloatingPanel();
            void build_header();
            bool on_drag_start(GdkEventButton *event);
            bool on_drag_motion(GdkEventMotion *event);
            bool on_drag_stop(GdkEventButton *event);
            void on_close_clicked();
            void set_panel_body(Gtk::Box& body) { _floating_panel.pack_start(body); }
        private:
            bool _dragging;
            double _drag_start_x, _drag_start_y;
            int _panel_start_x, _panel_start_y;
            Gtk::Box _header_bar;
            Gtk::Button _close_button;
            Gtk::Box _floating_panel;
    };
}