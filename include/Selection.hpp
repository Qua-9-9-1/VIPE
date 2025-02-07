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

    Selection() { clear(); };
    ~Selection() {};
    void                          set_type(int type);
    Type                          get_type() const;
    cv::Point                     get_start() { return _start; }
    cv::Point                     get_end() { return _end; }
    const std::vector<cv::Point>& get_points() { return _points; }
    cv::Mat&                      get_mask() { return _mask; }
    std::vector<cv::Rect>         get_resize_handles_points(const cv::Rect& selection,
                                                            int             handle_size = 10);
    std::vector<cv::Rect>         get_selections_rects() { return _selections_rects; }
    void                          update_selected_region(cv::Mat& image);
    cv::Rect&                     get_selected_region() { return _selected_region; }
    cv::Rect&                     get_last_rect() { return _selections_rects.back(); }
    ResizeHandle                  get_active_handle() { return _active_handle; }
    ResizeHandle                  detect_resize_handle(int x, int y);
    void set_active_handle(ResizeHandle handle) { _active_handle = handle; }
    void set_start(const cv::Point& start);
    void set_end(const cv::Point& end);
    void set_selections_rects(const std::vector<cv::Rect>& rects) { _selections_rects = rects; }
    void empty_unselected_mask();
    void normalize_selection();
    void update_region_from_rects();
    void set_selected_region(const cv::Rect& region) { _selected_region = region; }
    void move_selection_rects(int offset_x, int offset_y);
    void apply_resize_on_rects(float scale_x, float scale_y, const cv::Rect& new_bbox);
    void add_point(const cv::Point& point);
    void create_rect(int start_x, int start_y);
    void set_last_rect_end(int end_x, int end_y);
    void set_mask(const cv::Mat& mask) { _mask = mask; }
    void clear();
    bool is_selection_active() const;

  private:
    Type                   _type;
    cv::Point              _start;
    cv::Point              _end;
    std::vector<cv::Point> _points;
    cv::Rect               _selected_region;
    std::vector<cv::Rect>  _selections_rects;
    cv::Mat                _mask;
    ResizeHandle           _active_handle = ResizeHandle::None;
};
} // namespace vipe