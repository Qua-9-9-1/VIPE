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
#include "LayersPanel.hpp"
#include "ColorPalette.hpp"

namespace vipe {
class Menu;
class Canva;
class Toolkit;

class Engine : public Gtk::Window {
  public:
    Engine();
    ~Engine();
    void              build_menu();
    void              build_drawing_area();
    void              build_panels();
    Gtk::VBox&        get_vbox() { return _vbox; }
    Gtk::DrawingArea& get_drawing_area() { return _drawing_area; }
    bool              onDraw(const Cairo::RefPtr<Cairo::Context>& cr);
    // file options
    void create_file_option();
    void create_display_option();
    void create_image_option();
    void create_effect_option();
    void switch_canva(std::shared_ptr<Canva> canva);
    void new_file();
    void open_file();
    void open_file_from_path(const std::string& filename);
    void save_file();
    void save_as_file();
    void close_file();
    // display options
    void fullscreen();
    // image options
    void resize_image();
    // effect options
    void blur_effect();
    // events
    bool on_button_press(GdkEventButton* event);
    bool on_button_release(GdkEventButton* event);
    bool on_motion_notify(GdkEventMotion* event);
    bool on_scroll(GdkEventScroll* event);
    void canva_click_action(int x, int y, GdkEventButton* event);
    void canva_action(int x, int y);
    void brush_actions(int x, int y);
    bool on_key_press(GdkEventKey* event);
    bool key_events(GdkEventKey* event);
    bool file_shortcuts(GdkEventKey* event);
    bool canva_shortcuts(GdkEventKey* event);
    void on_window_resize(Gtk::Allocation& allocation);
    void update_menu_tool_types(std::vector<std::string> tool_types);

  private:
    Menu                                _menu;
    Gtk::VBox                           _vbox;
    Gtk::DrawingArea                    _drawing_area;
    std::shared_ptr<Canva>              _canva;
    std::vector<std::shared_ptr<Canva>> _canvas;
    sigc::connection                    _draw_connection;
    Gtk::Overlay                        _overlay;
    Gtk::Fixed                          _fixed_layout;
    std::unique_ptr<Toolkit>            _toolkit;
    std::unique_ptr<LayersPanel>        _layer_panel;
    std::shared_ptr<ColorPalette>       _color_palette;
};
} // namespace vipe