#include "Engine.hpp"
#include "FloatingPanel.hpp"

namespace vipe {
    MyWindow::MyWindow() : _menu(*this)
    {
        set_title("VIPE - Visual Image Processing Editor");
        set_default_size(800, 600);
        _vbox.set_orientation(Gtk::ORIENTATION_VERTICAL);
        add(_vbox);
        _canvas.push_back(Canva());
        _canva = std::make_shared<Canva>(_canvas[0]);
        _layer_panel = std::make_unique<LayersPanel>(*_canva);
        build_menu();
        build_drawing_area();
        build_panels();
        add_events(Gdk::KEY_PRESS_MASK);
        show_all_children();
    }

    MyWindow::~MyWindow() {}

    void MyWindow::build_menu()
    {
        auto screen = Gdk::Screen::get_default();
        auto style_context = Gtk::StyleContext::create();
        auto& menu_bar = _menu.get_menu_bar();
        auto& sub_menu = _menu.get_sub_menu();

        menu_bar.get_style_context()->add_class("menu-bar");
        _vbox.pack_start(menu_bar, Gtk::PACK_SHRINK);
        sub_menu.get_style_context()->add_class("sub-menu");
        _vbox.pack_start(sub_menu, Gtk::PACK_SHRINK);
    }

    void MyWindow::build_drawing_area()
    {
        _overlay.add(_drawing_area);
        _drawing_area.signal_draw().connect(sigc::mem_fun(*_canva, &Canva::display_canva));
        _drawing_area.add_events(Gdk::BUTTON_PRESS_MASK | Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_RELEASE_MASK);
        _drawing_area.signal_button_press_event().connect(sigc::mem_fun(*this, &MyWindow::on_button_press));
        _drawing_area.signal_button_release_event().connect(sigc::mem_fun(*this, &MyWindow::on_button_release));
        _drawing_area.signal_motion_notify_event().connect(sigc::mem_fun(*this, &MyWindow::on_motion_notify));
    }

    void MyWindow::build_panels()
    {

        _vbox.pack_start(_overlay, Gtk::PACK_EXPAND_WIDGET);
        _overlay.add_overlay(_fixed_layout);
        _fixed_layout.put(_layer_panel->get_layer_panel(), 300, 300);
        _fixed_layout.put(_toolkit.get_tool_panel(), 10, 10);
        _fixed_layout.put(_color_palette.get_color_palette(), 40, 200);
        _overlay.set_overlay_pass_through(_fixed_layout, true);
    }
}
