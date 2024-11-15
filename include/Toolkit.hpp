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
    pipette,
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
    int            get_current_tool() { return _current_tool; }
    void           build_tool_grid();
    void           on_tool_selected(Tool tool, Gtk::ToggleButton* clicked_button);
    FloatingPanel& get_tool_panel() { return _tool_panel; }

  private:
    Tool                            _current_tool;
    Gtk::Grid                       _icon_grid;
    std::vector<Gtk::ToggleButton*> _tool_buttons;
    Gtk::ToggleButton*              _last_selected_button;
    FloatingPanel                   _tool_panel;
};

} // namespace vipe
