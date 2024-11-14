#include "Toolkit.hpp"
#include <gtkmm/fixed.h>
#include <iostream>

namespace vipe {
Toolkit::Toolkit() : _current_tool(Tool::pencil), _tool_panel(10, 10) { build_tool_grid(); }

void Toolkit::build_tool_grid() {
    std::vector<std::string> tool_names = {"Pencil", "Brush",      "Eraser", "Pipette",   "Spray",
                                           "Bucket", "Text",       "Line",   "Rectangle", "Circle",
                                           "Lasso",  "Magic Wand", "Shape",  "Gradient"};
    std::vector<Tool>        tools = {pencil, brush,     eraser, pipette, spray,      bucket, text,
                                      line,   rectangle, circle, lasso,   magic_wand, shape,  gradient};
    _icon_grid.set_column_homogeneous(true);
    _icon_grid.set_row_homogeneous(true);
    for (size_t i = 0; i < tools.size(); ++i) {
        Gtk::Button* tool_button = Gtk::make_managed<Gtk::Button>(tool_names[i]);
        tool_button->signal_clicked().connect(
            [this, tool = tools[i]]() { on_tool_selected(tool); });
        _tool_buttons.push_back(tool_button);
        _icon_grid.attach(*tool_button, i % 2, i / 2);
    }
    Gtk::Box* tool_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    tool_box->pack_start(_icon_grid);
    _tool_panel.set_panel_body(*tool_box);
}

Toolkit::~Toolkit() {}

void Toolkit::on_tool_selected(Tool tool) { _current_tool = tool; }

} // namespace vipe