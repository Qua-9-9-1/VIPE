#pragma once

#include <gtkmm.h>
#include <vector>
#include <iostream>

#include "FloatingPanel.hpp"

namespace vipe {

    enum Tool {
        pencil,
        brush,
        eraser,
        spray,
        bucket,
        text,
        line,
        rectangle,
        circle,
        lasso,
        magic_wand,
        shape,
        gradient
    };

    class Toolkit {
    public:
        Toolkit();
        ~Toolkit();
        int get_current_tool() { return _current_tool; }
        void build_tool_grid();
        void on_tool_selected(Tool tool);
        FloatingPanel &get_tool_panel() { return _tool_panel; }

    private:
        Tool                        _current_tool;
        Gtk::Grid                   _icon_grid;
        std::vector<Gtk::Button*>   _tool_buttons;
        FloatingPanel               _tool_panel;
    };

}
