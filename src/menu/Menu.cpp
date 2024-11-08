#include "Menu.hpp"


namespace vipe {
    Menu::Menu(Engine& window) : _window(window), _tool_size(12)
    {
        _menu_bar.override_background_color(Gdk::RGBA("CAD7ED"));
        create_file_option();
        create_display_option();
        create_effect_option();
        create_image_option();
        _sub_menu.override_background_color(Gdk::RGBA("#DAE4F2"));
        create_sub_menu();
    }

    Menu::~Menu() {}

}
