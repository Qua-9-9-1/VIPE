#include "Menu.hpp"

namespace vipe {
Menu::Menu(Engine& engine)
    : _engine(engine), _tool_size(12), _tool_type(0), _tool_type_combo(nullptr),
      _selected_file_box(nullptr) {
    _menu_bar.override_background_color(Gdk::RGBA("CAD7ED"));
    create_file_option();
    create_display_option();
    create_effect_option();
    create_image_option();
    _sub_menu.override_background_color(Gdk::RGBA("#DAE4F2"));
    create_sub_menu();
    _bottom_bar.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    _bottom_bar.override_background_color(Gdk::RGBA("CAD7ED"));
    create_bottom_bar();
}

Menu::~Menu() {}

} // namespace vipe
