#include "Engine.hpp"

namespace vipe {

    bool Engine::on_button_press(GdkEventButton* event)
    {
        if (event->type == GDK_BUTTON_PRESS && event->button == 1) {
            _canva->set_color(cv::Scalar(0, 0, 0, 255));
        }
        if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
            _canva->set_color(cv::Scalar(255, 255, 255, 255));
        }
        return true;
    }

    bool Engine::on_button_release(GdkEventButton* event)
    {
        if (event->type == GDK_BUTTON_RELEASE
        && (event->button == 1 || event->button == 3)) {
            _canva->set_point_pos(-1, -1);
            _drawing_area.queue_draw();
        }
        return true;
    }

    bool Engine::on_motion_notify(GdkEventMotion* event)
    {
        if (event->state & (GDK_BUTTON1_MASK | GDK_BUTTON3_MASK)) {
            if (_toolkit.get_current_tool() == vipe::Tool::pencil) {
                _canva->draw_line(event->x, event->y, _menu.get_tool_size());
            } else if (_toolkit.get_current_tool() == vipe::Tool::eraser) {
                _canva->erase(event->x, event->y, _menu.get_tool_size());
            }
            _drawing_area.queue_draw();
        }
        return true;
    }

    bool Engine::file_shortcuts(GdkEventKey* event)
    {
        if ((event->state & GDK_CONTROL_MASK)
        && (event->keyval == GDK_KEY_n || event->keyval == GDK_KEY_N)) {
            new_file();
            return true;
        }
        if ((event->state & GDK_CONTROL_MASK)
        && (event->keyval == GDK_KEY_o || event->keyval == GDK_KEY_O)) {
            open_file();
            return true;
        }
        if ((event->state & GDK_CONTROL_MASK)
        && (event->keyval == GDK_KEY_s || event->keyval == GDK_KEY_S)) {
            save_file();
            return true;
        }
        if ((event->state & GDK_CONTROL_MASK)
        && (event->state & GDK_SHIFT_MASK)
        && (event->keyval == GDK_KEY_s || event->keyval == GDK_KEY_S)) {
            save_as_file();
            return true;
        }
        if ((event->state & GDK_CONTROL_MASK)
        && (event->keyval == GDK_KEY_w || event->keyval == GDK_KEY_W)) {
            close_file();
            return true;
        }
        return false;
    }

    bool Engine::on_key_press(GdkEventKey* event) {
        if (event->keyval == GDK_KEY_x || event->keyval == GDK_KEY_X) {
            std::cout << "X key pressed\n";
            return true;
        }
        if (file_shortcuts(event))
            return true;
        return Gtk::Window::on_key_press_event(event);
    }
}