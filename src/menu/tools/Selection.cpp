#include "Selection.hpp"

namespace vipe {

void Selection::set_type(int type) {
    switch (type) {
    case 0:
        _type = Type::Rectangular;
        break;
    case 1:
        _type = Type::Elliptical;
        break;
    case 2:
        _type = Type::Free;
        break;
    default:
        _type = Type::None;
        break;
    }
}

Selection::Type Selection::get_type() const { return _type; }

void Selection::set_start(const cv::Point& start) { _start = start; }

void Selection::set_end(const cv::Point& end) {
    _end = end;
    if (_start.x > _end.x)
        std::swap(_start.x, _end.x);
    if (_start.y > _end.y)
        std::swap(_start.y, _end.y);
}

void Selection::add_point(const cv::Point& point) { _points.push_back(point); }

void Selection::clear() {
    _points.clear();
    _start           = cv::Point(-1, -1);
    _end             = cv::Point(-1, -1);
    _mask            = cv::Mat();
    _selected_region = cv::Rect();
    _selections_rects.clear();
}

bool Selection::is_selection_active() const {
    return (!((_start.x == -1 || _start.y == -1 || _end.x == -1 || _end.y == -1) ||
              (_start.x == _end.x && _start.y == _end.y) || _selections_rects.empty()) ||
            !_points.empty());
}

std::vector<cv::Rect> Selection::get_resize_handles_points(const cv::Rect& selection,
                                                           int             handle_size) {
    return {cv::Rect(selection.x - handle_size / 2, selection.y - handle_size / 2, handle_size,
                     handle_size),
            cv::Rect(selection.x + selection.width - handle_size / 2, selection.y - handle_size / 2,
                     handle_size, handle_size),
            cv::Rect(selection.x - handle_size / 2,
                     selection.y + selection.height - handle_size / 2, handle_size, handle_size),
            cv::Rect(selection.x + selection.width - handle_size / 2,
                     selection.y + selection.height - handle_size / 2, handle_size, handle_size)};
}

void Selection::create_rect(int start_x, int start_y) {
    cv::Rect rect(start_x, start_y, 0, 0);
    _selections_rects.push_back(rect);
    update_region_from_rects();
    if (_selections_rects.size() == 1) {
        _start = cv::Point(start_x, start_y);
        _end   = cv::Point(start_x, start_y);
    }
}

void Selection::set_last_rect_end(int end_x, int end_y) {
    if (!_selections_rects.empty()) {
        _selections_rects.back().width  = end_x - _selections_rects.back().x;
        _selections_rects.back().height = end_y - _selections_rects.back().y;
    }
    update_region_from_rects();
}

Selection::ResizeHandle Selection::detect_resize_handle(int x, int y) {
    auto mask_rect = cv::Rect(_start, _end);
    auto handles   = get_resize_handles_points(mask_rect);

    for (size_t i = 0; i < handles.size(); i++) {
        if (handles[i].contains(cv::Point(x, y))) {
            return static_cast<ResizeHandle>(i + 1);
        }
    }
    return ResizeHandle::None;
}

void Selection::move_selection_rects(int offset_x, int offset_y) {
    for (auto& rect : _selections_rects) {
        rect.x += offset_x;
        rect.y += offset_y;
    }
    update_region_from_rects();
}

void Selection::apply_resize_on_rects(float sca_x, float sca_y, const cv::Rect& new_bbox) {
    auto  old_bbox = cv::Rect(_start, _end);
    float scale_x  = static_cast<float>(new_bbox.width) / old_bbox.width;
    float scale_y  = static_cast<float>(new_bbox.height) / old_bbox.height;

    for (auto& rect : _selections_rects) {
        // Position relative de chaque petit rectangle par rapport au grand rectangle
        float rel_x = static_cast<float>(rect.x - old_bbox.x) / old_bbox.width;
        float rel_y = static_cast<float>(rect.y - old_bbox.y) / old_bbox.height;
        float rel_w = static_cast<float>(rect.width) / old_bbox.width;
        float rel_h = static_cast<float>(rect.height) / old_bbox.height;

        // Nouvelle position et taille dans le cadre du nouveau rectangle englobant
        int new_x = new_bbox.x + rel_x * new_bbox.width;
        int new_y = new_bbox.y + rel_y * new_bbox.height;
        int new_w = std::max(1, static_cast<int>(rel_w * new_bbox.width));
        int new_h = std::max(1, static_cast<int>(rel_h * new_bbox.height));

        rect = cv::Rect(new_x, new_y, new_w, new_h);

        std::cout << "Updated rect: " << rect << std::endl;
    }
}

void Selection::normalize_selection() {
    // Normalisation des points de début et de fin pour éviter une sélection inversée.
    int x1 = std::min(_start.x, _end.x);
    int y1 = std::min(_start.y, _end.y);
    int x2 = std::max(_start.x, _end.x);
    int y2 = std::max(_start.y, _end.y);

    _start = cv::Point(x1, y1);
    _end   = cv::Point(x2, y2);

    // Normaliser le dernier rectangle si sa largeur ou hauteur est négative
    if (!_selections_rects.empty()) {
        auto& last_rect = _selections_rects.back();

        // Normaliser la largeur et la hauteur si elles sont négatives
        if (last_rect.width < 0) {
            last_rect.x += last_rect.width;
            last_rect.width = std::abs(last_rect.width);
        }

        if (last_rect.height < 0) {
            last_rect.y += last_rect.height;
            last_rect.height = std::abs(last_rect.height);
        }
    }
}

void Selection::empty_unselected_mask() {
    if (!is_selection_active() || _selected_region.area() <= 0) {
        _mask.release();
        return;
    }

    cv::Mat new_mask = cv::Mat::zeros(_selected_region.height, _selected_region.width, CV_8UC4);

    if (!_mask.empty()) {
        for (const auto& rect : _selections_rects) {
            cv::Rect old_roi(rect.x - _selected_region.x, rect.y - _selected_region.y, rect.width,
                             rect.height);
            cv::Rect new_roi(rect.x - _selected_region.x, rect.y - _selected_region.y, rect.width,
                             rect.height);
            old_roi = old_roi & cv::Rect(0, 0, _mask.cols, _mask.rows);
            new_roi = new_roi & cv::Rect(0, 0, new_mask.cols, new_mask.rows);

            if (old_roi.area() > 0 && new_roi.area() > 0) {
                _mask(old_roi).copyTo(new_mask(new_roi));
            }
        }
    }

    _mask = new_mask;
}

void Selection::update_region_from_rects() {
    if (!_selections_rects.empty()) {
        set_start(cv::Point(_selections_rects[0].x, _selections_rects[0].y));
        set_end(cv::Point(_selections_rects[0].x + _selections_rects[0].width,
                          _selections_rects[0].y + _selections_rects[0].height));
        for (const auto& rect : _selections_rects) {
            _start.x = std::min(_start.x, rect.x);
            _start.y = std::min(_start.y, rect.y);
            _end.x   = std::max(_end.x, rect.x + rect.width);
            _end.y   = std::max(_end.y, rect.y + rect.height);
        }
    }
}

void Selection::update_selected_region(cv::Mat& image) {
    if (is_selection_active()) {
        int x      = std::min(_start.x, _end.x);
        int y      = std::min(_start.y, _end.y);
        int width  = std::abs(_end.x - _start.x);
        int height = std::abs(_end.y - _start.y);

        // Vérifier que le rectangle ne dépasse pas les limites de l'image
        x      = std::max(0, x);
        y      = std::max(0, y);
        width  = std::min(width, image.cols - x);
        height = std::min(height, image.rows - y);

        _selected_region = cv::Rect(x, y, width, height);
    } else {
        _selected_region = cv::Rect(0, 0, image.cols, image.rows);
    }
}

} // namespace vipe
