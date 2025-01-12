#include "Canva.hpp"

namespace vipe {

void Canva::set_point_pos(int x, int y) {
    _prev_x = x;
    _prev_y = y;
}

void Canva::cursor_draw(int x1, int y1, int size) {
    bool is_selection_active = (_selection.is_selection_active() && !_selection.get_mask().empty());
    auto& drawing_zone =
        is_selection_active ? _selection.get_mask() : _layers[_selected_layer].image;

    apply_canva_drawing_factors(x1, y1);
    if (drawing_zone.empty()) {
        return;
    }
    if (drawing_zone.channels() == 4 && (_prev_x != -1 && _prev_y != -1)) {
        cv::Point prev_point(_prev_x - (is_selection_active ? _selected_region.x : 0),
                             _prev_y - (is_selection_active ? _selected_region.y : 0));
        cv::Point current_point(x1 - (is_selection_active ? _selected_region.x : 0),
                                y1 - (is_selection_active ? _selected_region.y : 0));
        cv::line(drawing_zone, prev_point, current_point, _color, size, cv::LINE_8);
        //  cv::LINE_AA for anti-aliasing
        if (!is_selection_active)
            mark_layer_for_update(_selected_layer);
    }
    set_point_pos(x1, y1);
}

void Canva::cursor_square(int x1, int y1, int size) {
    bool is_selection_active = (_selection.is_selection_active() && !_selection.get_mask().empty());
    auto& drawing_zone =
        is_selection_active ? _selection.get_mask() : _layers[_selected_layer].image;

    apply_canva_drawing_factors(x1, y1);
    if (drawing_zone.empty())
        return;

    if (_prev_x != -1 && _prev_y != -1) {
        float dist  = std::sqrt(std::pow(x1 - _prev_x, 2) + std::pow(y1 - _prev_y, 2));
        int   steps = std::max(1, static_cast<int>(dist / (size / 2)));

        for (int i = 0; i <= steps; ++i) {
            float     alpha = static_cast<float>(i) / steps;
            int       xi    = _prev_x + alpha * (x1 - _prev_x);
            int       yi    = _prev_y + alpha * (y1 - _prev_y);
            cv::Point prev_point((xi - size / 2) - (is_selection_active ? _selected_region.x : 0),
                                 (yi - size / 2) - (is_selection_active ? _selected_region.y : 0));
            cv::Point current_point(
                (xi + size / 2) - (is_selection_active ? _selected_region.x : 0),
                (yi + size / 2) - (is_selection_active ? _selected_region.y : 0));
            cv::rectangle(drawing_zone, prev_point, current_point, _color, cv::FILLED);
        }
        if (!is_selection_active)
            mark_layer_for_update(_selected_layer);
    }
    set_point_pos(x1, y1);
}

void Canva::cursor_triangle(int x1, int y1, int size) {
    bool is_selection_active = (_selection.is_selection_active() && !_selection.get_mask().empty());
    auto& drawing_zone =
        is_selection_active ? _selection.get_mask() : _layers[_selected_layer].image;

    apply_canva_drawing_factors(x1, y1);
    if (drawing_zone.empty())
        return;
    if (_prev_x != -1 && _prev_y != -1) {
        float dist  = std::sqrt(std::pow(x1 - _prev_x, 2) + std::pow(y1 - _prev_y, 2));
        int   steps = std::max(1, static_cast<int>(dist / (size / 2)));
        for (int i = 0; i <= steps; ++i) {
            float                  alpha  = static_cast<float>(i) / steps;
            int                    xi     = _prev_x + alpha * (x1 - _prev_x);
            int                    yi     = _prev_y + alpha * (y1 - _prev_y);
            std::vector<cv::Point> points = {
                cv::Point(xi - (is_selection_active ? _selected_region.x : 0),
                          (yi - size / 2) - (is_selection_active ? _selected_region.y : 0)),
                cv::Point((xi - size / 2) - (is_selection_active ? _selected_region.x : 0),
                          (yi + size / 2) - (is_selection_active ? _selected_region.y : 0)),
                cv::Point((xi + size / 2) - (is_selection_active ? _selected_region.x : 0),
                          (yi + size / 2) - (is_selection_active ? _selected_region.y : 0))};
            cv::fillConvexPoly(drawing_zone, points, _color);
        }
        if (!is_selection_active)
            mark_layer_for_update(_selected_layer);
    }
    set_point_pos(x1, y1);
}

void Canva::cursor_pastel(int x1, int y1, int size) {
    bool is_selection_active = (_selection.is_selection_active() && !_selection.get_mask().empty());
    auto& drawing_zone =
        is_selection_active ? _selection.get_mask() : _layers[_selected_layer].image;

    apply_canva_drawing_factors(x1, y1);
    if (drawing_zone.empty())
        return;

    int density = 20;
    for (int i = 0; i < density; ++i) {
        int       dx = (rand() % size) - size / 2;
        int       dy = (rand() % size) - size / 2;
        cv::Point point((x1 + dx) - (is_selection_active ? _selected_region.x : 0),
                        (y1 + dy) - (is_selection_active ? _selected_region.y : 0));
        cv::circle(drawing_zone, point, size, _color, cv::FILLED);
    }
    set_point_pos(x1, y1);
    if (!is_selection_active)
        mark_layer_for_update(_selected_layer);
}

void Canva::cursor_spray(int x1, int y1, int size) {
    bool is_selection_active = (_selection.is_selection_active() && !_selection.get_mask().empty());
    auto& drawing_zone =
        is_selection_active ? _selection.get_mask() : _layers[_selected_layer].image;

    apply_canva_drawing_factors(x1, y1);
    if (drawing_zone.empty())
        return;
    int num_particles = 30;
    int radius        = size / 2;
    for (int i = 0; i < num_particles; ++i) {
        int dx = (rand() % (2 * radius)) - radius;
        int dy = (rand() % (2 * radius)) - radius;
        if (dx * dx + dy * dy <= radius * radius) {
            drawing_zone.at<cv::Vec4b>((y1 + dy) - (is_selection_active ? _selected_region.x : 0),
                                       (x1 + dx) - (is_selection_active ? _selected_region.y : 0)) =
                _color;
        }
    }
    set_point_pos(x1, y1);
    if (!is_selection_active)
        mark_layer_for_update(_selected_layer);
}

void Canva::line_draw(int x, int y, int size) {
    bool is_selection_active = (_selection.is_selection_active() && !_selection.get_mask().empty());
    auto& drawing_zone =
        is_selection_active ? _selection.get_mask() : _layers[_selected_layer].image;

    apply_canva_drawing_factors(x, y);
    x = std::clamp(x, 0, drawing_zone.cols);
    y = std::clamp(y, 0, drawing_zone.rows);
    if (_prev_x == -1 && _prev_y == -1) {
        set_point_pos(x, y);
        return;
    }
    if (drawing_zone.empty())
        return;
    cv::Point prev_point(_prev_x - (is_selection_active ? _selected_region.x : 0),
                         _prev_y - (is_selection_active ? _selected_region.y : 0));
    cv::Point current_point(x - (is_selection_active ? _selected_region.x : 0),
                            y - (is_selection_active ? _selected_region.y : 0));

    cv::line(drawing_zone, prev_point, current_point, _color, size, cv::LINE_8);
    if (!is_selection_active)
        mark_layer_for_update(_selected_layer);
}

void Canva::color_fill() {
    auto& drawing_zone = _layers[_selected_layer].image;
    if (drawing_zone.empty())
        return;

    if (drawing_zone.channels() == 4 && (_prev_x != -1 && _prev_y != -1)) {
        cv::floodFill(drawing_zone, cv::Point(_prev_x, _prev_y), _color, 0, cv::Scalar(0, 0, 0, 0),
                      cv::Scalar(0, 0, 0, 0), cv::FLOODFILL_FIXED_RANGE);
        mark_layer_for_update(_selected_layer);
    }
}

cv::Scalar Canva::pick_color(int x, int y) {
    auto drawing_zone = _layers[_selected_layer].image;

    apply_canva_drawing_factors(x, y);
    if (x < 0 || x >= drawing_zone.cols || y < 0 || y >= drawing_zone.rows) {
        return cv::Scalar(0, 0, 0, 0);
    }
    if (drawing_zone.channels() == 4) {
        return drawing_zone.at<cv::Vec4b>(y, x);
    } else if (drawing_zone.channels() == 3) {
        cv::Vec3b color = drawing_zone.at<cv::Vec3b>(y, x);
        return cv::Vec4b(color[0], color[1], color[2], 255);
    }
    return cv::Scalar(0, 0, 0, 0);
}

} // namespace vipe