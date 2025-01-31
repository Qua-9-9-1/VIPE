#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <gtkmm.h>

namespace vipe {

class Selection {
  public:
    enum class Type { None, Rectangular, Elliptical, Free, Color };
    enum class ResizeHandle { None, TopLeft, TopRight, BottomLeft, BottomRight };

    Selection() {};
    ~Selection() {};
    void                          set_type(int type);
    Type                          get_type() const;
    cv::Point                     get_start() { return _start; }
    cv::Point                     get_end() { return _end; }
    const std::vector<cv::Point>& get_points() { return _points; }
    cv::Mat&                      get_mask() { return _mask; }
    std::vector<cv::Rect>         get_resize_handles_points(const cv::Rect& selection,
                                                            int             handle_size = 10);
    ResizeHandle                  get_active_handle() { return _active_handle; }
    ResizeHandle                  detect_resize_handle(int x, int y);
    void set_active_handle(ResizeHandle handle) { _active_handle = handle; }
    void set_start(const cv::Point& start);
    void set_end(const cv::Point& end);
    void add_point(const cv::Point& point);
    void set_mask(const cv::Mat& mask) { _mask = mask; }
    void clear();
    bool is_selection_active() const;

  private:
    Type                   _type;
    cv::Point              _start;
    cv::Point              _end;
    std::vector<cv::Point> _points;
    cv::Mat                _mask;
    ResizeHandle           _active_handle = ResizeHandle::None;
};
} // namespace vipe