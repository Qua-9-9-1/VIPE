#include "Canva.hpp"

namespace vipe {
void Canva::set_view_offset(int x, int y) {
    _view_offset_x = x;
    _view_offset_y = y;
}

void Canva::move_view(int x, int y, bool apply_zoom) {
    _view_offset_x += apply_zoom ? x / _zoom_factor : x;
    _view_offset_y += apply_zoom ? y / _zoom_factor : y;
}

void Canva::cursor_move_view(int x, int y) {
    move_view(x - _prev_x, y - _prev_y, false);
    _prev_x = x;
    _prev_y = y;
}

void Canva::set_zoom_factor(double factor) { _zoom_factor = factor; }

void Canva::zoom_view(double factor) { _zoom_factor += factor; }

void Canva::apply_canva_drawing_factors(int& x, int& y) {
    x = (x - _view_offset_x) / _zoom_factor;
    y = (y - _view_offset_y) / _zoom_factor;
}

void Canva::center_and_zoom_picture(int window_width, int window_height) {
    auto   picture_width  = _image.cols;
    auto   picture_height = _image.rows;
    double scale_width    = static_cast<double>(window_width) / picture_width;
    double scale_height   = static_cast<double>(window_height) / picture_height;
    double scale          = std::min(scale_width, scale_height) * 0.85;

    _zoom_factor   = scale;
    _view_offset_x = (window_width - picture_width * scale) / 2;
    _view_offset_y = (window_height - picture_height * scale) / 2;
    if (_view_offset_y < 0) {
        _view_offset_y = 0;
    }
}
} // namespace vipe