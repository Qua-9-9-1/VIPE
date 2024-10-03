#pragma once

#include <gtkmm.h>

namespace vipe {
    class MyWindow;

    class Menu {
        public:
            Menu(MyWindow& window);;
            ~Menu();
            Gtk::MenuBar& get_menu_bar() { return _menu_bar; }
            void create_file_option();
            void create_display_option();
            void create_effect_option();
            void create_image_option();
        private:
            MyWindow& _window;
            Gtk::MenuBar _menu_bar;
    };
}