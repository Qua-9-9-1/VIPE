#include "Engine.hpp"

namespace vipe {

bool Engine::on_button_press(GdkEventButton* event) {
    if (event->type == GDK_BUTTON_PRESS) {
        if ((event->button == 1 || event->button == 3)) {
            canva_click_action(event->x, event->y, event);
        } else if (event->button == 2) {
            _canva->set_point_pos(event->x, event->y);
        }
        _drawing_area.queue_draw();
        return true;
    }
    return false;
}

bool Engine::on_button_release(GdkEventButton* event) {
    if (event->type == GDK_BUTTON_RELEASE &&
        (event->button == 1 || event->button == 2 || event->button == 3)) {
        canva_release_action(event->x, event->y);
        _canva->set_point_pos(-1, -1);
        _drawing_area.queue_draw();
    }
    return true;
}

bool Engine::on_motion_notify(GdkEventMotion* event) {
    if (event->state & (GDK_BUTTON1_MASK | GDK_BUTTON3_MASK)) {
        canva_action(event->x, event->y);
        _drawing_area.queue_draw();
    } else if (event->state & GDK_BUTTON2_MASK) {
        _canva->cursor_move_view(event->x, event->y);
        _drawing_area.queue_draw();
    }
    return true;
}

void Engine::on_window_resize(Gtk::Allocation& allocation) {
    int new_width  = allocation.get_width();
    int new_height = allocation.get_height();

    // std::cout << "Window resized to " << new_width << "x" << new_height << std::endl;
    // _overlay.set_size_request(new_width, new_height);
    // _overlay.queue_resize();
}

void Engine::canva_click_action(int x, int y, GdkEventButton* event) {
    auto current_tool = _toolkit->get_current_tool();

    if (event) {
        if (event->button == 1) {
            _canva->set_color(_color_palette->get_first_color());
            if (current_tool == vipe::Tool::pipette) {
                _color_palette->set_first_color(_canva->pick_color(x, y));
            } else if (current_tool == vipe::Tool::eraser) {
                _canva->set_color(cv::Scalar(0, 0, 0, 0));
            }
            canva_one_click_action(x, y);
        } else if (event->button == 2) {
            _canva->set_point_pos(x, y);
        } else if (event->button == 3) {
            _canva->set_color(_color_palette->get_second_color());
            if (current_tool == vipe::Tool::pipette) {
                _color_palette->set_first_color(_canva->pick_color(x, y));
            } else if (current_tool == vipe::Tool::eraser) {
                _canva->set_color(cv::Scalar(0, 0, 0, 0));
            }
            canva_one_click_action(x, y);
        }
    }
}

void Engine::canva_one_click_action(int x, int y) {
    auto current_tool = _toolkit->get_current_tool();

    if (current_tool == vipe::Tool::pencil) {
        _canva->cursor_draw(x, y, 1);
        _canva->cursor_draw(x, y, 1);
    } else if (current_tool == vipe::Tool::brush || current_tool == vipe::Tool::eraser) {
        brush_actions(x, y);
        brush_actions(x, y);
    } else if (current_tool == vipe::Tool::bucket) {
        _canva->flood_fill(x, y);
    } else if (current_tool == vipe::Tool::cursor) {
        _canva->init_move_selection(x, y);
    } else if (current_tool == vipe::Tool::line) {
        _canva->line_draw(x, y, _menu.get_tool_size());
    } else if (current_tool == vipe::Tool::selection) {
        _canva->set_selection_start(x, y, _menu.get_tool_type());
    }
}

void Engine::canva_release_action(int x, int y) {
    auto current_tool = _toolkit->get_current_tool();

    if (current_tool == vipe::Tool::line) {
        _canva->line_draw(x, y, _menu.get_tool_size());
    } else if (current_tool == vipe::Tool::selection) {
        _canva->set_selection_end(x, y, _menu.get_tool_type());
    } else if (current_tool == vipe::Tool::cursor) {
        _canva->stop_selection_grab();
    }
}

void Engine::canva_action(int x, int y) {
    auto current_tool = _toolkit->get_current_tool();

    if (current_tool == vipe::Tool::pencil) {
        _canva->cursor_draw(x, y, 1);
    } else if (current_tool == vipe::Tool::brush || current_tool == vipe::Tool::eraser) {
        brush_actions(x, y);
    } else if (current_tool == vipe::Tool::cursor) {
        _canva->move_selection(x, y);
    } else if (current_tool == vipe::Tool::line) {
        // _canva->line_draw(x, y, _menu.get_tool_size());
    } else if (current_tool == vipe::Tool::selection) {
        _canva->resize_selection(x, y, _menu.get_tool_type());
    }
}

void Engine::brush_actions(int x, int y) {
    auto tool_type = _menu.get_tool_type();
    if (tool_type == 0) {
        _canva->cursor_draw(x, y, _menu.get_tool_size());
    } else if (tool_type == 1) {
        _canva->cursor_square(x, y, _menu.get_tool_size());
    } else if (tool_type == 2) {
        _canva->cursor_triangle(x, y, _menu.get_tool_size());
    } else if (tool_type == 3) {
        _canva->cursor_pastel(x, y, _menu.get_tool_size());
    } else if (tool_type == 4) {
        _canva->cursor_spray(x, y, _menu.get_tool_size());
    }
}

bool Engine::on_scroll(GdkEventScroll* event) {
    // bool is_ctrl  = event->state & GDK_CONTROL_MASK;
    // bool is_shift = event->state & GDK_SHIFT_MASK;

    if (event->direction == GDK_SCROLL_UP) {
        // if (is_ctrl) {
        //     _canva->zoom_view(0.1);
        // } else if (is_shift) {
        //     _canva->move_view(50, 0);
        // } else {
        _canva->move_view(0, -50);
        // }
        _drawing_area.queue_draw();
        return true;
    } else if (event->direction == GDK_SCROLL_DOWN) {
        // if (is_ctrl) {
        //     _canva->zoom_view(-0.1);
        // } else if (is_shift) {
        //     _canva->move_view(-50, 0);
        // } else {
        _canva->move_view(0, 50);
        // }
        _drawing_area.queue_draw();
        return true;
    }
    return false;
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
    if ((event->state & GDK_SHIFT_MASK) &&
        (event->keyval == GDK_KEY_s || event->keyval == GDK_KEY_S)) {
        save_as_file();
        return true;
    } else if ((event->keyval == GDK_KEY_s || event->keyval == GDK_KEY_S)) {
        save_file();
        return true;
    }
    if ((event->keyval == GDK_KEY_w || event->keyval == GDK_KEY_W)) {
        close_file();
        return true;
    }
    return false;
}

bool Engine::canva_shortcuts(GdkEventKey* event) {
    if ((event->keyval == GDK_KEY_b || event->keyval == GDK_KEY_B)) {
        _canva->center_and_zoom_picture(get_width(), get_height());
        _drawing_area.queue_draw();
        return true;
    }
    if ((event->keyval == GDK_KEY_c || event->keyval == GDK_KEY_C)) {
        _canva->copy_selection_to_clipboard();
        _drawing_area.queue_draw();
        return true;
    }
    if ((event->keyval == GDK_KEY_v || event->keyval == GDK_KEY_V)) {
        _canva->paste_from_clipboard();
        _toolkit->set_current_tool(vipe::Tool::cursor);
        _drawing_area.queue_draw();
        return true;
    }
    if ((event->keyval == GDK_KEY_x || event->keyval == GDK_KEY_X)) {
        _canva->cut_selection_to_clipboard();
        _drawing_area.queue_draw();
        return true;
    }
    return false;
}

bool Engine::key_events(GdkEventKey* event) {
    if ((event->keyval == GDK_KEY_Z || event->keyval == GDK_KEY_z)) {
        _canva->move_view(0, -50);
        return true;
    }
    if ((event->keyval == GDK_KEY_S || event->keyval == GDK_KEY_s)) {
        _canva->move_view(0, 50);
        return true;
    }
    if ((event->keyval == GDK_KEY_Q || event->keyval == GDK_KEY_q)) {
        _canva->move_view(-50, 0);
        return true;
    }
    if ((event->keyval == GDK_KEY_D || event->keyval == GDK_KEY_d)) {
        _canva->move_view(50, 0);
        return true;
    }
    if ((event->keyval == GDK_KEY_p || event->keyval == GDK_KEY_P) ||
        (event->keyval == GDK_KEY_KP_Add)) {
        zoom_on_canva(0.1);
        return true;
    }
    if ((event->keyval == GDK_KEY_m || event->keyval == GDK_KEY_M) ||
        (event->keyval == GDK_KEY_KP_Subtract)) {
        zoom_on_canva(-0.1);
        return true;
    }
    if ((event->keyval == GDK_KEY_b || event->keyval == GDK_KEY_B)) {
        if (_current_canva == 0) {
            _current_canva = _canvas.size() - 1;
        } else {
            --_current_canva;
        }
        switch_canva(_current_canva);
        return true;
    }
    if ((event->keyval == GDK_KEY_n || event->keyval == GDK_KEY_N)) {
        if (_current_canva == _canvas.size() - 1) {
            _current_canva = 0;
        } else {
            ++_current_canva;
        }
        switch_canva(_current_canva);
        return true;
    }
    if ((event->keyval == GDK_KEY_o || event->keyval == GDK_KEY_O)) {
        _toolkit->get_tool_panel().display_panel();
        _layer_panel->get_layer_panel().display_panel();
        _color_palette->get_color_palette().display_panel();
        return true;
    }
    return false;
}

bool Engine::on_key_press(GdkEventKey* event) {
    if ((event->state & GDK_CONTROL_MASK) && file_shortcuts(event))
        return true;
    if ((event->state & GDK_CONTROL_MASK) && canva_shortcuts(event))
        return true;
    if (key_events(event)) {
        _drawing_area.queue_draw();
        return true;
    }
    return Gtk::Window::on_key_press_event(event);
}
} // namespace vipe