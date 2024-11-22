#include "Engine.hpp"

namespace vipe {

void Engine::new_file() {
    _canvas.push_back(Canva());
    _canva = std::make_shared<Canva>(_canvas.back());
    _canva->create_blank_picture(800, 600);
    _canva->center_and_zoom_picture(get_width(), get_height());
    _drawing_area.queue_draw();
}

void Engine::open_file() {
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
        open_file_from_path(filename);
    }
}

void Engine::open_file_from_path(const std::string& filename) {
    _canva->set_image(filename);
    _canva->set_filename(filename);
    _canva->center_and_zoom_picture(get_width(), get_height());
    _drawing_area.queue_draw();
}

void Engine::save_file() {
    auto filename = _canva->get_filename();
    auto image    = _canva->get_merged_image();

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

void Engine::save_as_file() {
    auto image = _canva->get_merged_image();

    if (image.empty()) {
        std::cerr << "Aucune image à sauvegarder." << std::endl;
        return;
    }

    Gtk::FileChooserDialog dialog("Enregistrer l'image", Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.set_transient_for(*this);
    dialog.add_button("_Annuler", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Enregistrer", Gtk::RESPONSE_OK);
    Gtk::Box          format_box(Gtk::ORIENTATION_HORIZONTAL);
    Gtk::Label        format_label("Format :");
    Gtk::ComboBoxText format_selector;
    format_selector.append("PNG");
    format_selector.append("JPEG");
    format_selector.set_active(0);
    format_box.pack_start(format_label, Gtk::PACK_SHRINK);
    format_box.pack_start(format_selector, Gtk::PACK_EXPAND_WIDGET);
    format_box.show_all();
    dialog.set_extra_widget(format_box);

    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK) {
        std::string filename = dialog.get_filename();
        if (filename.empty()) {
            Gtk::MessageDialog error_dialog("Erreur : Le nom du fichier est vide.", false,
                                            Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            error_dialog.run();
            return;
        }
        std::string format = format_selector.get_active_text();
        if (format == "PNG" && filename.find(".png") == std::string::npos) {
            filename += ".png";
        } else if (format == "JPEG" && filename.find(".jpg") == std::string::npos &&
                   filename.find(".jpeg") == std::string::npos) {
            filename += ".jpg";
        }
        _canva->set_filename(filename);
        if (format == "PNG") {
            cv::imwrite(_canva->get_filename(), image, {cv::IMWRITE_PNG_COMPRESSION, 3});
        } else if (format == "JPEG") {
            cv::imwrite(_canva->get_filename(), image, {cv::IMWRITE_JPEG_QUALITY, 90});
        }
    }
}

void Engine::close_file() {
    return;
    _canvas.pop_back();
    _canva = std::make_shared<Canva>(_canvas.back());
    _drawing_area.queue_draw();
}
} // namespace vipe