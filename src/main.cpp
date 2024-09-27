#include <gtkmm.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

class MyWindow : public Gtk::Window {
public:
    MyWindow() {
        set_title("VIPE - Visual Image Processing Environment");
        set_default_size(800, 600);
        
        add(drawing_area);
        show_all_children();
    }

protected:
    Gtk::DrawingArea drawing_area;
    
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override {
        cv::Mat img = cv::imread("test_image.jpg", cv::IMREAD_COLOR);
        if (img.empty()) {
            std::cerr << "Erreur lors du chargement de l'image !" << std::endl;
            return false;
        }
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
        Gdk::Cairo::set_source_pixbuf(cr, Gdk::Pixbuf::create_from_data(
            img.data, Gdk::COLORSPACE_RGB, false, 8, img.cols, img.rows, img.step));
        cr->paint();        
        return true;
    }
};

int main(int const argc, char const **argv) {
    auto app = Gtk::Application::create(argc, argv, "org.gtkmm.example");

    MyWindow window;
    return app->run(window);
}
