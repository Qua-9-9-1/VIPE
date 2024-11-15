#include "FloatingPanel.hpp"

namespace vipe {
FloatingPanel::FloatingPanel(int pos_x, int pos_y, bool important)
    : Gtk::Box(Gtk::ORIENTATION_VERTICAL), _dragging(false), _drag_start_x(0), _drag_start_y(0),
      _panel_start_x(pos_x), _panel_start_y(pos_y), _important(important), _close_button(nullptr),
      _current_body(nullptr) {
    override_background_color(Gdk::RGBA("lightblue"));
    build_header();
    pack_start(_floating_panel, Gtk::PACK_SHRINK);
    set_halign(Gtk::ALIGN_START);
    set_valign(Gtk::ALIGN_START);
    set_margin_start(_panel_start_x);
    set_margin_top(_panel_start_y);
}

FloatingPanel::~FloatingPanel() {}

void FloatingPanel::build_header() {
    auto event_box = Gtk::make_managed<Gtk::EventBox>();
    _close_button  = Gtk::make_managed<Gtk::Button>();

    _floating_panel.set_orientation(Gtk::ORIENTATION_VERTICAL);
    _header_bar.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    _close_button->set_image_from_icon_name("window-close-symbolic", Gtk::ICON_SIZE_BUTTON);
    _close_button->signal_clicked().connect(sigc::mem_fun(*this, &FloatingPanel::hide_panel));
    _header_bar.pack_end(*_close_button, Gtk::PACK_SHRINK);
    event_box->add(_header_bar);
    pack_start(*event_box, Gtk::PACK_SHRINK);
    event_box->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
                          Gdk::POINTER_MOTION_MASK);
    event_box->signal_button_press_event().connect(
        sigc::mem_fun(*this, &FloatingPanel::on_drag_start));
    event_box->signal_motion_notify_event().connect(
        sigc::mem_fun(*this, &FloatingPanel::on_drag_motion));
    event_box->signal_button_release_event().connect(
        sigc::mem_fun(*this, &FloatingPanel::on_drag_stop));
    event_box->override_background_color(Gdk::RGBA("lightsteelblue"));
    _floating_panel.pack_start(_header_bar, Gtk::PACK_SHRINK);
}

void FloatingPanel::hide_panel() { hide(); }

void FloatingPanel::display_panel() { show_all(); }

bool FloatingPanel::on_drag_start(GdkEventButton* event) {
    if (event->button == 1) {
        _dragging     = true;
        _drag_start_x = event->x_root;
        _drag_start_y = event->y_root;
    }
    return true;
}

bool FloatingPanel::on_drag_motion(GdkEventMotion* event) {
    if (_dragging) {
        int new_x = _panel_start_x + (event->x_root - _drag_start_x);
        int new_y = _panel_start_y + (event->y_root - _drag_start_y);
        set_margin_start(new_x);
        set_margin_top(new_y);
    }
    queue_draw();
    return true;
}

bool FloatingPanel::on_drag_stop(GdkEventButton* event) {
    if (event->button == 1) {
        _dragging = false;
        _panel_start_x += (event->x_root - _drag_start_x);
        _panel_start_y += (event->y_root - _drag_start_y);
    }
    return true;
}

void FloatingPanel::set_panel_body(Gtk::Box& body) {
    for (auto* child : _floating_panel.get_children()) {
        _floating_panel.remove(*child);
    }
    _floating_panel.pack_start(body, Gtk::PACK_SHRINK);
    _floating_panel.show_all();
}

void FloatingPanel::clear_panel_body() {
    // _floating_panel.remove()
}

} // namespace vipe