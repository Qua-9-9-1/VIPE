#include <gtkmm.h>
#include <iostream>

enum tools {
    pen,
    pencil,
    eraser,
    brush,
    spray,
    paint_bucket,
    text,
    line,
    rectangle,
    circle,
    lasso,
    magic_wand,
    shape,
    gradient
};

namespace vipe {
    class Toolkit {
        public:
            Toolkit();
            ~Toolkit();
            static void load_css(const std::string& path) {
                auto css_provider = Gtk::CssProvider::create();
                css_provider->load_from_path(path);

                auto screen = Gdk::Screen::get_default();
                auto style_context = Gtk::StyleContext::create();
                style_context->add_provider_for_screen(screen, css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
            }
        private:
            tools _current_tool;
            bool  _display_popup;
            
    };
}