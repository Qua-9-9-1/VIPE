#pragma once

#include <gtkmm.h>

namespace vipe {
class Engine;

class Menu {
  public:
    Menu(Engine& engine);
    ;
    ~Menu();
    Gtk::MenuBar& get_menu_bar() { return _menu_bar; }
    Gtk::VBox&    get_sub_menu() { return _sub_menu; }
    Gtk::Box&     get_bottom_bar() { return _bottom_bar; }
    int           get_tool_size() { return _tool_size; }
    int           get_tool_type() { return _tool_type; }
    void          create_file_option();
    void          create_display_option();
    void          create_effect_option();
    void          create_image_option();
    void          create_sub_menu();
    void          create_bottom_bar();
    void          add_open_file(const std::string& file_name);
    void          on_file_clicked(int index);
    void          remove_open_file(Gtk::EventBox* file_event_box);
    void          clear_selection();
    void          remove_open_file(int index);
    void          update_open_file_name(int index, const std::string& file_name);
    void          update_tool_types(std::vector<std::string> tool_types);

  private:
    int                         _tool_size;
    Engine&                     _engine;
    int                         _tool_type;
    bool                        _smooth;
    Gtk::MenuBar                _menu_bar;
    Gtk::Box                    _canvas_widget;
    Gtk::Box                    _bottom_bar;
    Gtk::Box*                   _open_files_box;
    Gtk::VBox                   _sub_menu;
    Gtk::ComboBoxText*          _tool_type_combo;
    Gtk::EventBox*              _selected_file_box;
    std::vector<Gtk::EventBox*> _file_boxes;
};
} // namespace vipe