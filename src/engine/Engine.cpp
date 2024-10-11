#include "Engine.hpp"

namespace vipe {
    MyWindow::MyWindow(): _menu(*this), _pen(true)
    {
        set_title("VIPE - Visual Image Processing Editor");
        set_default_size(800, 600);
        _vbox.set_orientation(Gtk::ORIENTATION_VERTICAL);
        add(_vbox);
        _canvas.push_back(Canva());
        // _canva = std::make_shared<Canva>(_canvas.back());
        _canva = std::make_shared<Canva>();

        auto css_provider = Gtk::CssProvider::create();
        css_provider->load_from_path("../src/styles/Menu.css");

        auto screen = Gdk::Screen::get_default();
        auto style_context = Gtk::StyleContext::create();
        style_context->add_provider_for_screen(screen, css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);

        auto& menu_bar = _menu.get_menu_bar();
        menu_bar.get_style_context()->add_class("menu-bar");
        _vbox.pack_start(menu_bar, Gtk::PACK_SHRINK);

        _drawing_area.signal_draw().connect(sigc::mem_fun(*_canva, &Canva::display_canva));
        _drawing_area.add_events(Gdk::BUTTON_PRESS_MASK | Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_RELEASE_MASK);
        _drawing_area.signal_button_press_event().connect(sigc::mem_fun(*this, &MyWindow::on_button_press));
        _drawing_area.signal_button_release_event().connect(sigc::mem_fun(*this, &MyWindow::on_button_release));
        _drawing_area.signal_motion_notify_event().connect(sigc::mem_fun(*this, &MyWindow::on_motion_notify));
        add_events(Gdk::KEY_PRESS_MASK);
        _vbox.pack_start(_drawing_area);

        show_all_children();
    }

    MyWindow::~MyWindow() {}
}
