#include "FloatingPanel.hpp"

namespace vipe {
    FloatingPanel::FloatingPanel()
        : Gtk::Box(Gtk::ORIENTATION_VERTICAL),
        _dragging(false),
        _drag_start_x(0), _drag_start_y(0),
        _panel_start_x(10), _panel_start_y(10)
    {
        override_background_color(Gdk::RGBA("lightblue"));
        build_header();
        pack_start(_floating_panel, Gtk::PACK_SHRINK);
        _floating_panel.show_all_children();
    }

    FloatingPanel::~FloatingPanel() {}

    void FloatingPanel::build_header()
    {
        auto event_box = Gtk::make_managed<Gtk::EventBox>();

        _floating_panel.set_orientation(Gtk::ORIENTATION_VERTICAL);
        _header_bar.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        _close_button.set_label("x");
        _close_button.signal_clicked().connect(sigc::mem_fun(*this, &FloatingPanel::on_close_clicked));
        _header_bar.pack_end(_close_button, Gtk::PACK_SHRINK);    
        event_box->add(_header_bar);
        pack_start(*event_box, Gtk::PACK_SHRINK);
        event_box->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
        event_box->signal_button_press_event().connect(sigc::mem_fun(*this, &FloatingPanel::on_drag_start));
        event_box->signal_motion_notify_event().connect(sigc::mem_fun(*this, &FloatingPanel::on_drag_motion));
        event_box->signal_button_release_event().connect(sigc::mem_fun(*this, &FloatingPanel::on_drag_stop));
        _floating_panel.pack_start(_header_bar, Gtk::PACK_SHRINK);
    }

    void FloatingPanel::on_close_clicked() {
        _floating_panel.hide();
        _close_button.signal_button_press_event().connect([](GdkEventButton* event) {
            return true;
        });
    }

    bool FloatingPanel::on_drag_start(GdkEventButton *event) {
        if (event->button == 1) {
            _dragging = true;
            _drag_start_x = event->x_root;
            _drag_start_y = event->y_root;
        }
        return true;
    }

    bool FloatingPanel::on_drag_motion(GdkEventMotion *event) {
        if (_dragging) {
            int new_x = _panel_start_x + (event->x_root - _drag_start_x);
            int new_y = _panel_start_y + (event->y_root - _drag_start_y);

            Gtk::Widget* parent = get_parent();
            if (auto* fixed_layout = dynamic_cast<Gtk::Fixed*>(parent)) {
                fixed_layout->move(*this, new_x, new_y);
            }
        }
        return true;
    }

    bool FloatingPanel::on_drag_stop(GdkEventButton *event) {
        if (event->button == 1) {
            _dragging = false;
            _panel_start_x += (event->x_root - _drag_start_x);
            _panel_start_y += (event->y_root - _drag_start_y);
        }
        return true;
    }
}