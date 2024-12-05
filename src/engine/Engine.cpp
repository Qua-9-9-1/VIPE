#include "Engine.hpp"
#include "FloatingPanel.hpp"

namespace vipe {
Engine::Engine()
    : _menu(*this), _canva(nullptr), _toolkit(nullptr), _layer_panel(nullptr),
      _color_palette(nullptr) {
    set_title("VIPE - Visual Image Processing Editor");
    set_default_size(800, 600);
    try {
        if (set_icon_from_file("../assets/logo.png")) {
            std::cout << "Icône chargée avec succès." << std::endl;
        } else {
            std::cerr << "L'icône n'a pas pu être chargée." << std::endl;
        }
    } catch (const Glib::FileError& ex) {
        std::cerr << "Erreur lors du chargement de l'icône : " << ex.what() << std::endl;
    } catch (const Gdk::PixbufError& ex) {
        std::cerr << "Erreur lors du traitement de l'image : " << ex.what() << std::endl;
    }
    _vbox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    add(_vbox);
    _toolkit = std::make_unique<Toolkit>(*this);
    new_file();
    _color_palette = std::make_shared<ColorPalette>();
    _layer_panel   = std::make_unique<LayersPanel>(*_canva, _drawing_area);
    signal_key_press_event().connect(sigc::mem_fun(*this, &Engine::on_key_press));
    signal_size_allocate().connect(sigc::mem_fun(*this, &Engine::on_window_resize));

    build_menu();
    build_drawing_area();
    build_panels();
    add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);
    grab_focus();
    update_menu_tool_types(_toolkit->get_tool_type());
    // open_file_from_path("/home/quentin_d2/Pictures/testing.png");
    show_all_children();
    _layer_panel->on_close_options_panel();
    _canva->center_and_zoom_picture(800, 600);
}

Engine::~Engine() {}

void Engine::build_menu() {
    auto  screen        = Gdk::Screen::get_default();
    auto  style_context = Gtk::StyleContext::create();
    auto& menu_bar      = _menu.get_menu_bar();
    auto& sub_menu      = _menu.get_sub_menu();
    auto& bottom_bar    = _menu.get_bottom_bar();

    _vbox.pack_start(menu_bar, Gtk::PACK_SHRINK);
    _vbox.pack_start(sub_menu, Gtk::PACK_SHRINK);
    _vbox.pack_end(bottom_bar, Gtk::PACK_SHRINK);
}

void Engine::build_drawing_area() {
    _overlay.add(_drawing_area);
    _overlay.set_hexpand(true);
    _overlay.set_vexpand(true);
    _draw_connection =
        _drawing_area.signal_draw().connect(sigc::mem_fun(*_canva, &Canva::display_canva));
    _drawing_area.add_events(Gdk::SCROLL_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::POINTER_MOTION_MASK |
                             Gdk::BUTTON_RELEASE_MASK);
    _drawing_area.signal_button_press_event().connect(
        sigc::mem_fun(*this, &Engine::on_button_press));
    _drawing_area.signal_button_release_event().connect(
        sigc::mem_fun(*this, &Engine::on_button_release));
    _drawing_area.signal_motion_notify_event().connect(
        sigc::mem_fun(*this, &Engine::on_motion_notify));
    _drawing_area.signal_scroll_event().connect(sigc::mem_fun(*this, &Engine::on_scroll));
}

void Engine::build_panels() {
    _vbox.pack_start(_overlay, Gtk::PACK_EXPAND_WIDGET);
    _overlay.add_overlay(_layer_panel->get_layer_panel());
    _overlay.add_overlay(_color_palette->get_color_palette());
    _overlay.add_overlay(_toolkit->get_tool_panel());
    _overlay.add_overlay(_layer_panel->get_options_panel());
}

void Engine::update_menu_tool_types(std::vector<std::string> tool_types) {
    _menu.update_tool_types(tool_types);
}

void Engine::switch_canva(std::shared_ptr<Canva> canva) {
    if (!canva) {
        std::cerr << "Aucun canva à charger." << std::endl;
        _canva = nullptr;
        _drawing_area.queue_draw();
        return;
    }
    _canva = canva;
    if (_layer_panel != nullptr) {
        _layer_panel->set_canva(*canva);
    }
    if (_draw_connection.connected()) {
        _draw_connection.disconnect();
    }
    _draw_connection =
        _drawing_area.signal_draw().connect(sigc::mem_fun(*_canva, &Canva::display_canva));
    _drawing_area.queue_draw();
}

void Engine::zoom_on_canva(float coef) {
    _canva->zoom_view(coef);
    _drawing_area.queue_draw();
}

} // namespace vipe
