#pragma once

#include <gtkmm.h>
#include <iostream>

namespace vipe {

class Engine;

class FloatingPanel : public Gtk::Box {
  public:
    FloatingPanel(int pos_x, int pos_y, bool important = false);
    ~FloatingPanel();
    void build_header();
    bool on_drag_start(GdkEventButton* event);
    bool on_drag_motion(GdkEventMotion* event);
    bool on_drag_stop(GdkEventButton* event);
    void hide_panel();
    void display_panel();
    void set_panel_body(Gtk::Box& body);
    void clear_panel_body();

  private:
    bool         _important;
    bool         _dragging;
    double       _drag_start_x, _drag_start_y;
    int          _panel_start_x, _panel_start_y;
    Gtk::Box     _header_bar;
    Gtk::Button  _close_button;
    Gtk::Box     _floating_panel;
    Gtk::Widget* _current_body = nullptr;
};
} // namespace vipe