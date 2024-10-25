#include "Menu.hpp"


namespace vipe {
    Menu::Menu(MyWindow& window) : _window(window), _tool_size(1)
    {
        _menu_bar.override_background_color(Gdk::RGBA("D5E1F5"));
        create_file_option();
        create_display_option();
        create_effect_option();
        create_image_option();
        _sub_menu.override_background_color(Gdk::RGBA("#DAE4F2"));
        create_sub_menu();
    }

    Menu::~Menu() {}

}
