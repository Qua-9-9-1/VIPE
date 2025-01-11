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
        cv::Mat temp_layer = layer.clone();
        cv::line(temp_layer, cv::Point(_prev_x, _prev_y), cv::Point(x1, y1), _color, size,
                 cv::LINE_8);
        //  cv::LINE_AA for anti-aliasing
        cv::Mat mask_roi = _selected_region(cv::Rect(0, 0, layer.cols, layer.rows));
        temp_layer.copyTo(layer, mask_roi);
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
        cv::Mat temp_layer = layer.clone();
        float   dist       = std::sqrt(std::pow(x1 - _prev_x, 2) + std::pow(y1 - _prev_y, 2));
        int     steps      = std::max(1, static_cast<int>(dist / (size / 2)));
        for (int i = 0; i <= steps; ++i) {
            float alpha = static_cast<float>(i) / steps;
            int   xi    = _prev_x + alpha * (x1 - _prev_x);
            int   yi    = _prev_y + alpha * (y1 - _prev_y);
            cv::rectangle(temp_layer, cv::Point(xi - size / 2, yi - size / 2),
                          cv::Point(xi + size / 2, yi + size / 2), _color, cv::FILLED);
        }
        cv::Mat mask_roi = _selected_region(cv::Rect(0, 0, layer.cols, layer.rows));
        temp_layer.copyTo(layer, mask_roi);
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
        cv::Mat temp_layer = layer.clone();
        float   dist       = std::sqrt(std::pow(x1 - _prev_x, 2) + std::pow(y1 - _prev_y, 2));
        int     steps      = std::max(1, static_cast<int>(dist / (size / 2)));
        for (int i = 0; i <= steps; ++i) {
            float                  alpha  = static_cast<float>(i) / steps;
            int                    xi     = _prev_x + alpha * (x1 - _prev_x);
            int                    yi     = _prev_y + alpha * (y1 - _prev_y);
            std::vector<cv::Point> points = {cv::Point(xi, yi - size / 2),
                                             cv::Point(xi - size / 2, yi + size / 2),
                                             cv::Point(xi + size / 2, yi + size / 2)};
            cv::fillConvexPoly(temp_layer, points, _color);
        }
        cv::Mat mask_roi = _selected_region(cv::Rect(0, 0, layer.cols, layer.rows));
        temp_layer.copyTo(layer, mask_roi);
    }
    set_point_pos(x1, y1);
    mark_layer_for_update(_selected_layer);
}

void Canva::cursor_pastel(int x1, int y1, int size) {
    auto& layer = _layers[_selected_layer].image;

    apply_canva_drawing_factors(x1, y1);
    if (layer.empty())
        return;

    cv::Mat temp_layer = layer.clone();
    int     density    = 20;
    for (int i = 0; i < density; ++i) {
        int dx = (rand() % size) - size / 2;
        int dy = (rand() % size) - size / 2;
        cv::circle(temp_layer, cv::Point(x1 + dx, y1 + dy), 1, _color, cv::FILLED);
    }
    cv::Mat mask_roi = _selected_region(cv::Rect(0, 0, layer.cols, layer.rows));
    temp_layer.copyTo(layer, mask_roi);
    set_point_pos(x1, y1);
    mark_layer_for_update(_selected_layer);
}

void Canva::cursor_spray(int x1, int y1, int size) {
    auto& layer = _layers[_selected_layer].image;

    apply_canva_drawing_factors(x1, y1);
    if (layer.empty())
        return;

    cv::Mat temp_layer    = layer.clone();
    int     num_particles = 30;
    int     radius        = size / 2;
    for (int i = 0; i < num_particles; ++i) {
        int dx = (rand() % (2 * radius)) - radius;
        int dy = (rand() % (2 * radius)) - radius;
        if (dx * dx + dy * dy <= radius * radius) {
            temp_layer.at<cv::Vec4b>(y1 + dy, x1 + dx) = _color;
        }
    }
    cv::Mat mask_roi = _selected_region(cv::Rect(0, 0, layer.cols, layer.rows));
    temp_layer.copyTo(layer, mask_roi);
    set_point_pos(x1, y1);
    mark_layer_for_update(_selected_layer);
}

void Canva::line_draw(int x, int y, int size) {
    auto& layer = _layers[_selected_layer].image;

    apply_canva_drawing_factors(x, y);
    x = std::clamp(x, 0, layer.cols);
    y = std::clamp(y, 0, layer.rows);
    if (_prev_x == -1 && _prev_y == -1) {
        set_point_pos(x, y);
        return;
    }
    if (layer.empty())
        return;
    cv::Mat temp_layer = layer.clone();
    cv::line(temp_layer, cv::Point(_prev_x, _prev_y), cv::Point(x, y), _color, size, cv::LINE_8);
    cv::Mat mask_roi = _selected_region(cv::Rect(0, 0, layer.cols, layer.rows));
    temp_layer.copyTo(layer, mask_roi);

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

void Canva::set_selection_start(int x, int y, int type) {
    _selection.clear();
    update_selected_region();
    _selection.set_type(type);
    apply_canva_drawing_factors(x, y);
    x = std::clamp(x, 0, _image.cols);
    y = std::clamp(y, 0, _image.rows);
    _selection.set_start(cv::Point(x, y));
}

void Canva::resize_selection(int x, int y, int type) {
    // type = 2 == lasso, type = 3 == magic_wand
    apply_canva_drawing_factors(x, y);
    x = std::clamp(x, 0, _image.cols);
    y = std::clamp(y, 0, _image.rows);
    if (type == 2)
        _selection.add_point(cv::Point(x, y));
    else if (type == 3)
        return;
    else
        _selection.set_end(cv::Point(x, y));
}

void Canva::set_selection_end(int x, int y, int type) {
    // type = 2 == lasso, type = 3 == magic_wand
    apply_canva_drawing_factors(x, y);
    x = std::clamp(x, 0, _image.cols);
    y = std::clamp(y, 0, _image.rows);
    if (type == 2)
        _selection.add_point(cv::Point(x, y));
    else if (type == 3)
        return;
    else
        _selection.set_end(cv::Point(x, y));
    // if (!_selection.is_selection_active())
    //     _selection.clear();
    update_selected_region();
}

} // namespace vipe