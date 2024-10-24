#include "Menu.hpp"


namespace vipe {
    Menu::Menu(MyWindow& window) : _window(window), _tool_size(1)
    {
        create_file_option();
        create_display_option();
        create_effect_option();
        create_image_option();
        create_sub_menu();
    }

    Menu::~Menu() {}

}
