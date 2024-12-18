#include "Canva.hpp"

namespace vipe {

void Canva::set_point_pos(int x, int y) {
    _prev_x = x;
    _prev_y = y;
}

void Canva::cursor_draw(int x1, int y1, int size) {
    auto& layer = _layers[_selected_layer].image;

    apply_canva_drawing_factors(x1, y1);
    if (layer.empty()) {
        return;
    }
    if (layer.channels() == 4 && (_prev_x != -1 && _prev_y != -1)) {
        cv::line(layer, cv::Point(_prev_x, _prev_y), cv::Point(x1, y1), _color, size);
    }
    set_point_pos(x1, y1);
    mark_layer_for_update(_selected_layer);
}

void Canva::cursor_square(int x1, int y1, int size) {
    auto& layer = _layers[_selected_layer].image;

    apply_canva_drawing_factors(x1, y1);
    if (layer.empty())
        return;

    if (_prev_x != -1 && _prev_y != -1) {
        float dist  = std::sqrt(std::pow(x1 - _prev_x, 2) + std::pow(y1 - _prev_y, 2));
        int   steps = std::max(1, static_cast<int>(dist / (size / 2)));
        for (int i = 0; i <= steps; ++i) {
            float alpha = static_cast<float>(i) / steps;
            int   xi    = _prev_x + alpha * (x1 - _prev_x);
            int   yi    = _prev_y + alpha * (y1 - _prev_y);
            cv::rectangle(layer, cv::Point(xi - size / 2, yi - size / 2),
                          cv::Point(xi + size / 2, yi + size / 2), _color, cv::FILLED);
        }
    }
    set_point_pos(x1, y1);
    mark_layer_for_update(_selected_layer);
}

void Canva::cursor_triangle(int x1, int y1, int size) {
    auto& layer = _layers[_selected_layer].image;

    apply_canva_drawing_factors(x1, y1);
    if (layer.empty())
        return;
    if (_prev_x != -1 && _prev_y != -1) {
        float dist  = std::sqrt(std::pow(x1 - _prev_x, 2) + std::pow(y1 - _prev_y, 2));
        int   steps = std::max(1, static_cast<int>(dist / (size / 2)));
        for (int i = 0; i <= steps; ++i) {
            float                  alpha  = static_cast<float>(i) / steps;
            int                    xi     = _prev_x + alpha * (x1 - _prev_x);
            int                    yi     = _prev_y + alpha * (y1 - _prev_y);
            std::vector<cv::Point> points = {cv::Point(xi, yi - size / 2),
                                             cv::Point(xi - size / 2, yi + size / 2),
                                             cv::Point(xi + size / 2, yi + size / 2)};
            cv::fillConvexPoly(layer, points, _color);
        }
    }
    set_point_pos(x1, y1);
    mark_layer_for_update(_selected_layer);
}

void Canva::cursor_pastel(int x1, int y1, int size) {
    auto& layer = _layers[_selected_layer].image;

    apply_canva_drawing_factors(x1, y1);
    if (layer.empty())
        return;

    int density = 20;
    for (int i = 0; i < density; ++i) {
        int dx = (rand() % size) - size / 2;
        int dy = (rand() % size) - size / 2;
        cv::circle(layer, cv::Point(x1 + dx, y1 + dy), 1, _color, cv::FILLED);
    }
    set_point_pos(x1, y1);
    mark_layer_for_update(_selected_layer);
}

void Canva::cursor_spray(int x1, int y1, int size) {
    auto& layer = _layers[_selected_layer].image;

    apply_canva_drawing_factors(x1, y1);
    if (layer.empty())
        return;

    int num_particles = 30;
    int radius        = size / 2;
    for (int i = 0; i < num_particles; ++i) {
        int dx = (rand() % (2 * radius)) - radius;
        int dy = (rand() % (2 * radius)) - radius;
        if (dx * dx + dy * dy <= radius * radius) {
            layer.at<cv::Vec4b>(y1 + dy, x1 + dx) = _color;
        }
    }
    set_point_pos(x1, y1);
    mark_layer_for_update(_selected_layer);
}

void Canva::line_draw(int x, int y, int size) {
    auto& layer = _layers[_selected_layer].image;
    apply_canva_drawing_factors(x, y);
    if (_prev_x == -1 && _prev_y == -1) {
        set_point_pos(x, y);
        return;
    }

    if (layer.empty())
        return;
    cv::line(layer, cv::Point(_prev_x, _prev_y), cv::Point(x, y), _color, size);
    mark_layer_for_update(_selected_layer);
}

void Canva::color_fill() {
    auto& layer = _layers[_selected_layer].image;
    if (layer.empty())
        return;

    if (layer.channels() == 4 && (_prev_x != -1 && _prev_y != -1)) {
        cv::floodFill(layer, cv::Point(_prev_x, _prev_y), _color, 0, cv::Scalar(0, 0, 0, 0),
                      cv::Scalar(0, 0, 0, 0), cv::FLOODFILL_FIXED_RANGE);
    }
    mark_layer_for_update(_selected_layer);
}

cv::Scalar Canva::pick_color(int x, int y) {
    auto layer = _layers[_selected_layer].image;

    apply_canva_drawing_factors(x, y);
    if (x < 0 || x >= layer.cols || y < 0 || y >= layer.rows) {
        return cv::Scalar(0, 0, 0, 0);
    }
    if (layer.channels() == 4) {
        return layer.at<cv::Vec4b>(y, x);
    } else if (layer.channels() == 3) {
        cv::Vec3b color = layer.at<cv::Vec3b>(y, x);
        return cv::Vec4b(color[0], color[1], color[2], 255);
    }
    return cv::Scalar(0, 0, 0, 0);
}
} // namespace vipe