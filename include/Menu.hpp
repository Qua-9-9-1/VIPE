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
    void          update_tool_types(std::vector<std::string> tool_types);

  private:
    int                _tool_size;
    Engine&            _engine;
    int                _tool_type;
    Gtk::MenuBar       _menu_bar;
    Gtk::Box           _bottom_bar;
    Gtk::VBox          _sub_menu;
    Gtk::ComboBoxText* _tool_type_combo;
};
} // namespace vipe