#include "Engine.hpp"

namespace vipe {

bool Engine::on_button_press(GdkEventButton* event) {
    if (event->type == GDK_BUTTON_PRESS && (event->button == 1 || event->button == 3)) {
        canva_click_action(event->x, event->y, event);
    }
    return true;
}

bool Engine::on_button_release(GdkEventButton* event) {
    if (event->type == GDK_BUTTON_RELEASE && (event->button == 1 || event->button == 3)) {
        _canva->set_point_pos(-1, -1);
        _drawing_area.queue_draw();
    }
    return true;
}

bool Engine::on_motion_notify(GdkEventMotion* event) {
    if (event->state & (GDK_BUTTON1_MASK | GDK_BUTTON3_MASK)) {
        canva_action(event->x, event->y);
    }
    return true;
}

void Engine::canva_click_action(int x, int y, GdkEventButton* event) {
    if (event && event->button == 1) {
        _canva->set_color(_color_palette->get_first_color());
    } else if (event && event->button == 3) {
        _canva->set_color(_color_palette->get_second_color());
    }

    auto current_tool = _toolkit.get_current_tool();

    if (event && current_tool == vipe::Tool::pipette) {
        if (event->button == 1) {
            _color_palette->set_first_color(_canva->pick_color(x, y));
        } else if (event->button == 3) {
            _color_palette->set_second_color(_canva->pick_color(x, y));
        }
        return;
    }
    canva_action(x, y);
}

void Engine::canva_action(int x, int y) {
    auto current_tool = _toolkit.get_current_tool();

    if (current_tool == vipe::Tool::pencil) {
        _canva->cursor_draw(x, y, 1);
    } else if (current_tool == vipe::Tool::brush) {
        _canva->cursor_draw(x, y, _menu.get_tool_size());
    } else if (current_tool == vipe::Tool::eraser) {
        _canva->cursor_erase(x, y, _menu.get_tool_size());
    } else if (current_tool == vipe::Tool::bucket) {
        _canva->cursor_square(x, y, _menu.get_tool_size());
    } else if (current_tool == vipe::Tool::line) {
        _canva->cursor_triangle(x, y, _menu.get_tool_size());
    } else if (current_tool == vipe::Tool::rectangle) {
        _canva->cursor_pastel(x, y, _menu.get_tool_size());
    } else if (current_tool == vipe::Tool::circle) {
        _canva->cursor_spray(x, y, _menu.get_tool_size());
    } else if (current_tool == vipe::Tool::lasso) {
        _canva->color_fill(0);
    }

    _drawing_area.queue_draw();
}

bool Engine::file_shortcuts(GdkEventKey* event) {
    if ((event->keyval == GDK_KEY_n || event->keyval == GDK_KEY_N)) {
        new_file();
        return true;
    }
    if ((event->keyval == GDK_KEY_o || event->keyval == GDK_KEY_O)) {
        open_file();
        return true;
    }
    if ((event->keyval == GDK_KEY_s || event->keyval == GDK_KEY_S)) {
        save_file();
        return true;
    }
    if ((event->state & GDK_SHIFT_MASK) &&
        (event->keyval == GDK_KEY_s || event->keyval == GDK_KEY_S)) {
        save_as_file();
        return true;
    }
    if ((event->keyval == GDK_KEY_w || event->keyval == GDK_KEY_W)) {
        close_file();
        return true;
    }
    return false;
}

bool Engine::key_events(GdkEventKey* event) {
    if ((event->keyval == GDK_KEY_Z || event->keyval == GDK_KEY_z)) {
        _canva->move_view(0, -10);
        return true;
    }
    if ((event->keyval == GDK_KEY_S || event->keyval == GDK_KEY_s)) {
        _canva->move_view(0, 10);
        return true;
    }
    if ((event->keyval == GDK_KEY_Q || event->keyval == GDK_KEY_q)) {
        _canva->move_view(-10, 0);
        return true;
    }
    if ((event->keyval == GDK_KEY_D || event->keyval == GDK_KEY_d)) {
        _canva->move_view(10, 0);
        return true;
    }
    return false;
}

bool Engine::on_key_press(GdkEventKey* event) {
    if ((event->state & GDK_CONTROL_MASK) && file_shortcuts(event))
        return true;
    if (key_events(event)) {
        _drawing_area.queue_draw();
        return true;
    }
    return Gtk::Window::on_key_press_event(event);
}
} // namespace vipe