#include "Menu.hpp"


namespace vipe {
    Menu::Menu(MyWindow& window) : _window(window)
    {
        create_file_option();
        create_display_option();
        create_effect_option();
        create_image_option();
    }

    Menu::~Menu() {}

}
