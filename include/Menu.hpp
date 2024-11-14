#pragma once

#include <gtkmm.h>

namespace vipe {
class Engine;

class Menu {
  public:
    Menu(Engine& window);
    ;
    ~Menu();
    Gtk::MenuBar& get_menu_bar() { return _menu_bar; }
    Gtk::VBox&    get_sub_menu() { return _sub_menu; }
    int           get_tool_size() { return _tool_size; }
    void          create_file_option();
    void          create_display_option();
    void          create_effect_option();
    void          create_image_option();
    void          create_sub_menu();

  private:
    int          _tool_size;
    Engine&      _window;
    Gtk::MenuBar _menu_bar;
    Gtk::VBox    _sub_menu;
};
} // namespace vipe