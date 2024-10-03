#include "Engine.hpp"

namespace vipe {

    void MyWindow::new_file()
    {
        _current_image.release();
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
            std::cout << "Fichier sélectionné : " << filename << std::endl;

            _current_image = cv::imread(filename, cv::IMREAD_COLOR);
            if (!_current_image.empty()) {
                _drawing_area.queue_draw();
            } else {
                std::cerr << "Erreur lors du chargement de l'image." << std::endl;
            }
        }
    }

    void MyWindow::save_file()
    {
        if (_current_image.empty()) {
            std::cerr << "Aucune image à sauvegarder." << std::endl;
            return;
        }

        if (_current_filename.empty()) {
            save_as_file();
            return;
        }

        cv::imwrite(_current_filename, _current_image);
    }

    void MyWindow::save_as_file()
    {
        if (_current_image.empty()) {
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
            std::cout << "Fichier enregistré : " << filename << std::endl;

            cv::imwrite(filename, _current_image);
            _current_filename = filename;
        }
    }

    void MyWindow::close_file()
    {
        _current_image.release();
        _current_filename.clear();
        _drawing_area.queue_draw();
    }
}