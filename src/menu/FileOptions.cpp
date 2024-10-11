#include "Engine.hpp"

namespace vipe {

    void MyWindow::new_file()
    {
        //ajouter un canva vierge au vecteur
        _canvas.push_back(Canva());
        _canva = std::make_shared<Canva>(_canvas.back());
        _drawing_area.queue_draw();
    }

    void MyWindow::open_file()
    {
        Gtk::FileChooserDialog dialog("Choisir une image", Gtk::FILE_CHOOSER_ACTION_OPEN);
        dialog.set_transient_for(*this);
        dialog.add_button("_Annuler", Gtk::RESPONSE_CANCEL);
        dialog.add_button("_Ouvrir", Gtk::RESPONSE_OK);
        auto filter_image = Gtk::FileFilter::create();
        filter_image->set_name("Images");
        filter_image->add_mime_type("image/jpeg");
        filter_image->add_mime_type("image/png");
        dialog.add_filter(filter_image);
        int result = dialog.run();
        if (result == Gtk::RESPONSE_OK) {
            std::string filename = dialog.get_filename();
            _canva->set_image(filename);
            _canva->set_filename(filename);
            _drawing_area.queue_draw();
        }
    }

    void MyWindow::save_file()
    {
        auto image = _canva->get_image();
        auto filename = _canva->get_filename();

        if (image.empty()) {
            std::cerr << "Aucune image à sauvegarder." << std::endl;
            return;
        }
        if (filename.empty()) {
            save_as_file();
            return;
        }
        cv::imwrite(filename, image);
    }

    void MyWindow::save_as_file()
    {
        auto image = _canva->get_image();

        if (image.empty()) {
            std::cerr << "Aucune image à sauvegarder." << std::endl;
            return;
        }

        Gtk::FileChooserDialog dialog("Enregistrer l'image", Gtk::FILE_CHOOSER_ACTION_SAVE);
        dialog.set_transient_for(*this);
        dialog.add_button("_Annuler", Gtk::RESPONSE_CANCEL);
        dialog.add_button("_Enregistrer", Gtk::RESPONSE_OK);

        auto filter_image = Gtk::FileFilter::create();
        filter_image->set_name("Images");
        filter_image->add_mime_type("image/jpeg");
        filter_image->add_mime_type("image/png");
        dialog.add_filter(filter_image);

        int result = dialog.run();

        if (result == Gtk::RESPONSE_OK) {
            std::string filename = dialog.get_filename();

            _canva->set_filename(filename);
            cv::imwrite(_canva->get_filename(), image);
        }
    }

    void MyWindow::close_file()
    {
        //supprimer le canva courant du vecteur
        _canvas.pop_back();
        _canva = std::make_shared<Canva>(_canvas.back());
        _drawing_area.queue_draw();
    }
}