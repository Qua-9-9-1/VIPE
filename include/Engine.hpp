#include <gtkmm.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

namespace vipe {
    class MyWindow : public Gtk::Window {
        public:
            MyWindow();
            ~MyWindow();
            bool onDraw(const Cairo::RefPtr<Cairo::Context>& cr);
        private:
            Gtk::DrawingArea drawing_area;
    };
}