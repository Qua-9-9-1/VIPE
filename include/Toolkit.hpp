#pragma once

#include <gtkmm.h>
#include <vector>
#include <iostream>

#include "Engine.hpp"
#include "FloatingPanel.hpp"

namespace vipe {

enum Tool { pencil, brush, eraser, pipette, bucket, text, line, selection, shape, gradient };

class Toolkit {
  public:
    Toolkit(Engine& engine);
    ~Toolkit();
    int                      get_current_tool() { return _current_tool; }
    void                     build_tool_grid();
    void                     on_tool_selected(Tool tool, Gtk::ToggleButton* clicked_button);
    FloatingPanel&           get_tool_panel() { return _tool_panel; }
    std::vector<std::string> get_tool_type();

  private:
    Engine&                         _engine;
    Tool                            _current_tool;
    Gtk::Grid                       _icon_grid;
    std::vector<Gtk::ToggleButton*> _tool_buttons;
    Gtk::ToggleButton*              _last_selected_button;
    FloatingPanel                   _tool_panel;
};

} // namespace vipe
