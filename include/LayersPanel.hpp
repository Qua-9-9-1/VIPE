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
    LayersPanel(Canva& canva, Gtk::DrawingArea& drawing_area);
    ~LayersPanel();
    void           set_canva(Canva& canva);
    FloatingPanel& get_layer_panel() { return _layer_panel; }
    FloatingPanel& get_options_panel() { return _layer_options; }

    void build_layer_panel();
    void build_options_panel();
    void update_layer_list();
    void update_options_panel();
    void clear_layer_list();
    void add_layer_to_list(int i);
    void add_visibility_button(Gtk::Box* layer_box, int i);
    void add_options_button(Gtk::Box* layer_box, int i);
    void add_delete_button(Gtk::Box* layer_box, int i);
    void add_add_layer_button();
    void add_layer_up_button(int index);
    void add_layer_down_button(int index);
    void add_image_layer_button();

    void on_layer_clicked(int index, Gtk::Box* clicked_layer);
    bool on_layer_drag_motion(int x, int y);
    void on_add_layer();
    void on_move_layer_up(int index);
    void on_move_layer_down(int index);
    void on_add_layer_from_image();
    void toggle_layer_visibility(int index, Gtk::Button* visibility_button);
    void set_layer_options_panel();
    void set_layer_mode(int mode);
    void on_delete_layer(int index);
    void on_validate_options_panel(int opacity);
    void on_close_options_panel();

  private:
    Canva&                 _canva;
    Gtk::DrawingArea&      _drawing_area;
    std::vector<Gtk::Box*> _layers;
    Gtk::Box*              _last_selected_layer;
    Gtk::Box               _layer_list_container;
    FloatingPanel          _layer_panel;
    Gtk::Box               _layer_options_container;
    FloatingPanel          _layer_options;
    Gtk::Scale*            _opacity_scale;
    Gtk::ComboBoxText*     _blend_mode_combo;
};
} // namespace vipe