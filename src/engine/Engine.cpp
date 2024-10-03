#include "Engine.hpp"

namespace vipe {
    MyWindow::MyWindow(): _menu(*this)
    {
        set_title("VIPE - Visual Image Processing Editor");
        set_default_size(800, 600);
        _vbox.set_orientation(Gtk::ORIENTATION_VERTICAL);
        add(_vbox);

        auto css_provider = Gtk::CssProvider::create();
        css_provider->load_from_path("../src/styles/Menu.css");

        auto screen = Gdk::Screen::get_default();
        auto style_context = Gtk::StyleContext::create();
        style_context->add_provider_for_screen(screen, css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);

        auto& menu_bar = _menu.get_menu_bar();
        menu_bar.get_style_context()->add_class("menu-bar");
        _vbox.pack_start(menu_bar, Gtk::PACK_SHRINK);

        _drawing_area.signal_draw().connect(sigc::mem_fun(*this, &MyWindow::onDraw));
        _vbox.pack_start(_drawing_area);

        show_all_children();
    }

    MyWindow::~MyWindow() {}

    bool MyWindow::onDraw(const Cairo::RefPtr<Cairo::Context>& cr)
    {
        if (_current_image.empty()) {
            return false;
        }

        cv::Mat img_rgb;
        cv::cvtColor(_current_image, img_rgb, cv::COLOR_BGR2RGB);

        auto pixbuf = Gdk::Pixbuf::create_from_data(
            img_rgb.data, Gdk::COLORSPACE_RGB, false, 8,
            img_rgb.cols, img_rgb.rows, img_rgb.step);

        Gdk::Cairo::set_source_pixbuf(cr, pixbuf, 0, 0);
        cr->paint();
        return true;
    }
}
