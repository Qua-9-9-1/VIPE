#include "Engine.hpp"

namespace vipe {
    MyWindow::MyWindow() {
        set_title("VIPE - Visual Image Processing Editor");
        set_default_size(800, 600);
        set_position(Gtk::WIN_POS_CENTER);
        show_all_children();
    }

    MyWindow::~MyWindow() {}

    bool MyWindow::onDraw(const Cairo::RefPtr<Cairo::Context>& cr) {
        cv::Mat img = cv::imread("../assets/bg.png", cv::IMREAD_COLOR);
        
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
}