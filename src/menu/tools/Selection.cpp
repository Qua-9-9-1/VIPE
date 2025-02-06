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

void Selection::set_end(const cv::Point& end) { _end = end; }

void Selection::add_point(const cv::Point& point) { _points.push_back(point); }

void Selection::clear() {
    _points.clear();
    _start = cv::Point(-1, -1);
    _end   = cv::Point(-1, -1);
    _mask  = cv::Mat();
}

bool Selection::is_selection_active() const {
    return ((_start.x != -1 || _start.y != -1 || _end.x != -1 || _end.y != -1) &&
            (_start.x != _end.x && _start.y != _end.y)) ||
           !_points.empty();
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

Selection::ResizeHandle Selection::detect_resize_handle(int x, int y) {
    auto mask_rect = cv::Rect(_start, _end);
    auto handles   = get_resize_handles_points(mask_rect);

    std::cout << "Handles size: " << handles.size() << std::endl;
    for (size_t i = 0; i < handles.size(); i++) {
        std::cout << "Handle " << i << ": " << handles[i] << std::endl;
        std::cout << "X: " << x << " Y: " << y << std::endl;
        if (handles[i].contains(cv::Point(x, y))) {
            std::cout << "Handle " << i << " selected" << std::endl;
            return static_cast<ResizeHandle>(i + 1);
        }
    }
    return ResizeHandle::None;
}

} // namespace vipe
