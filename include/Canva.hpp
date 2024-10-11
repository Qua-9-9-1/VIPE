#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <gtkmm.h>
#include <iostream>

namespace vipe {
    class Canva {
        public:
            Canva();
            ~Canva();
            bool display_canva(const Cairo::RefPtr<Cairo::Context>& cr);
            void on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
            void set_image(const std::string& filename);
            void set_filename(const std::string& filename) { _current_filename = filename; }
            void set_point_pos(int x, int y) { _prev_x = x; _prev_y = y; }
            void set_color(cv::Scalar color) { _color = color; }
            std::string get_filename() { return _current_filename; }
            cv::Mat get_image() { return _image; }
            void draw_line(int x1, int y1);
            void erase(int x1, int y1);
            void recalculate_background(const cv::Size& new_size);
            void convert_to_RGBA(const cv::Mat& src, cv::Mat& dst);
            Cairo::RefPtr<Cairo::ImageSurface> create_cairo_surface(const cv::Mat& image);
        private:
            cv::Mat             _background;
            cv::Mat             _bg_tiled;
            cv::Mat             _image;
            std::string         _current_filename;
            int                 _prev_x;
            int                 _prev_y;
            cv::Scalar          _color;
    };
}
