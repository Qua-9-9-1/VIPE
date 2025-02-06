#include "Menu.hpp"
#include "Engine.hpp"

namespace vipe {
void Menu::create_file_option() {
    auto file_menu = Gtk::make_managed<Gtk::Menu>();
    auto file_item = Gtk::make_managed<Gtk::MenuItem>("Fichier");
    file_item->set_submenu(*file_menu);

    auto new_item = Gtk::make_managed<Gtk::MenuItem>("Nouveau");
    new_item->signal_activate().connect(sigc::mem_fun(_engine, &Engine::new_file));
    file_menu->append(*new_item);

    auto open_item = Gtk::make_managed<Gtk::MenuItem>("Ouvrir");
    open_item->signal_activate().connect(sigc::mem_fun(_engine, &Engine::open_file));
    file_menu->append(*open_item);

    auto save_item = Gtk::make_managed<Gtk::MenuItem>("Enregistrer");
    save_item->signal_activate().connect(sigc::mem_fun(_engine, &Engine::save_file));
    file_menu->append(*save_item);

    auto save_as_item = Gtk::make_managed<Gtk::MenuItem>("Enregistrer sous");
    save_as_item->signal_activate().connect(sigc::mem_fun(_engine, &Engine::save_as_file));
    file_menu->append(*save_as_item);

    auto close_item = Gtk::make_managed<Gtk::MenuItem>("Fermer");
    close_item->signal_activate().connect(sigc::mem_fun(_engine, &Engine::close_file));
    file_menu->append(*close_item);

    _menu_bar.append(*file_item);
}

void Menu::create_display_option() {
    auto display_menu = Gtk::make_managed<Gtk::Menu>();
    auto display_item = Gtk::make_managed<Gtk::MenuItem>("Affichage");
    display_item->set_submenu(*display_menu);

    auto fullscreen_item = Gtk::make_managed<Gtk::MenuItem>("Plein écran");
    fullscreen_item->signal_activate().connect(sigc::mem_fun(_engine, &Engine::fullscreen));
    display_menu->append(*fullscreen_item);

    _menu_bar.append(*display_item);
}

void Menu::create_image_option() {
    auto image_menu = Gtk::make_managed<Gtk::Menu>();
    auto image_item = Gtk::make_managed<Gtk::MenuItem>("Image");
    image_item->set_submenu(*image_menu);

    auto resize_item = Gtk::make_managed<Gtk::MenuItem>("Redimensionner");
    resize_item->signal_activate().connect(sigc::mem_fun(_engine, &Engine::resize_image));
    image_menu->append(*resize_item);

    _menu_bar.append(*image_item);
}

void Menu::create_effect_option() {
    auto effect_menu = Gtk::make_managed<Gtk::Menu>();
    auto effect_item = Gtk::make_managed<Gtk::MenuItem>("Effet");
    effect_item->set_submenu(*effect_menu);

    auto blur_item = Gtk::make_managed<Gtk::MenuItem>("Flou");
    blur_item->signal_activate().connect(sigc::mem_fun(_engine, &Engine::blur_effect));
    effect_menu->append(*blur_item);

    _menu_bar.append(*effect_item);
}

void Menu::create_sub_menu() {
    _open_files_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    _open_files_box->set_spacing(5);

    auto label                = Gtk::make_managed<Gtk::Label>("Option de l'outil");
    auto brush_size_scale     = Gtk::make_managed<Gtk::Scale>();
    auto horizontal_box       = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    auto toggle_smooth_button = Gtk::make_managed<Gtk::Button>("Lissage: ON");

    brush_size_scale->set_range(1.0, 400.0);
    brush_size_scale->set_value(_tool_size);
    brush_size_scale->signal_value_changed().connect([this, brush_size_scale]() {
        _tool_size = static_cast<int>(brush_size_scale->get_value());
    });

    toggle_smooth_button->signal_clicked().connect([this, toggle_smooth_button]() {
        _smooth = !_smooth;
        toggle_smooth_button->set_label(_smooth ? "Lissage: ON" : "Lissage: OFF");
    });

    _tool_type_combo = Gtk::make_managed<Gtk::ComboBoxText>();
    horizontal_box->pack_start(*_tool_type_combo, Gtk::PACK_SHRINK);
    horizontal_box->pack_start(*brush_size_scale, Gtk::PACK_EXPAND_WIDGET);
    horizontal_box->pack_start(*toggle_smooth_button, Gtk::PACK_SHRINK);

    _sub_menu.set_orientation(Gtk::ORIENTATION_VERTICAL);
    _sub_menu.pack_start(*_open_files_box, Gtk::PACK_SHRINK);
    _sub_menu.pack_start(*label, Gtk::PACK_SHRINK);
    _sub_menu.pack_start(*horizontal_box, Gtk::PACK_SHRINK);
}

void Menu::add_open_file(const std::string& file_name) {
    auto file_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    file_box->set_size_request(150, 50);
    clear_selection();
    file_box->override_background_color(Gdk::RGBA("#E0E0FF"));

    auto file_event_box = Gtk::make_managed<Gtk::EventBox>();
    file_event_box->add(*file_box);

    _file_boxes.push_back(file_event_box);

    file_event_box->signal_button_press_event().connect(
        [this, file_event_box](GdkEventButton*) -> bool {
            clear_selection();

            auto it = std::find(_file_boxes.begin(), _file_boxes.end(), file_event_box);
            if (it != _file_boxes.end()) {
                int index = std::distance(_file_boxes.begin(), it);

                on_file_clicked(index);
            }

            auto file_box = dynamic_cast<Gtk::Box*>(file_event_box->get_child());
            if (file_box) {
                file_box->override_background_color(Gdk::RGBA("#E0E0FF"));
            }
            _selected_file_box = file_event_box;
            return true;
        });

    auto file_label = Gtk::make_managed<Gtk::Label>(file_name);
    file_label->set_ellipsize(Pango::ELLIPSIZE_END);

    auto close_button = Gtk::make_managed<Gtk::Button>("X");
    close_button->set_size_request(20, 20);
    close_button->signal_clicked().connect(
        [this, file_event_box]() { remove_open_file(file_event_box); });

    file_box->pack_start(*file_label, Gtk::PACK_EXPAND_WIDGET);
    file_box->pack_end(*close_button, Gtk::PACK_SHRINK);
    _selected_file_box = file_event_box;

    _open_files_box->pack_start(*file_event_box, Gtk::PACK_SHRINK);
    _open_files_box->show_all_children();
}

void Menu::clear_selection() {
    if (_selected_file_box) {
        auto selected_box = dynamic_cast<Gtk::Box*>(_selected_file_box->get_child());
        if (selected_box) {
            selected_box->override_background_color(Gdk::RGBA("lightgray"));
        }
        _selected_file_box = nullptr;
    }
}

void Menu::on_file_clicked(int index) { _engine.switch_canva(index); }

void Menu::remove_open_file(int index) {
    if (index >= 0 && index < _file_boxes.size()) {
        _open_files_box->remove(*_file_boxes[index]);
        _file_boxes.erase(_file_boxes.begin() + index);
        _open_files_box->show_all_children();
    }
}

void Menu::remove_open_file(Gtk::EventBox* file_event_box) {
    auto it = std::find(_file_boxes.begin(), _file_boxes.end(), file_event_box);
    if (it != _file_boxes.end()) {
        int index = std::distance(_file_boxes.begin(), it);
        _engine.close_file_from_id(index);
    }
    auto new_selection = _engine.get_current_canva_index();
    clear_selection();
    _file_boxes[new_selection]->get_child()->override_background_color(Gdk::RGBA("#E0E0FF"));
}

void Menu::update_open_file_name(int index, const std::string& file_name) {
    if (index >= 0 && index < _file_boxes.size()) {
        auto file_box = dynamic_cast<Gtk::Box*>(_file_boxes[index]->get_child());
        if (file_box) {
            auto file_label = dynamic_cast<Gtk::Label*>(file_box->get_children().front());
            if (file_label) {
                file_label->set_text(file_name);
            }
        }
    }
}

void Menu::create_bottom_bar() {
    auto widget = Gtk::make_managed<Gtk::Label>("VIPE");
    _bottom_bar.pack_start(*widget, Gtk::PACK_SHRINK);
    auto zoom_in_button = Gtk::make_managed<Gtk::Button>();
    zoom_in_button->set_image_from_icon_name("zoom-in-symbolic", Gtk::ICON_SIZE_BUTTON);
    zoom_in_button->signal_clicked().connect([this] { _engine.zoom_on_canva(0.1); });
    _bottom_bar.pack_end(*zoom_in_button, Gtk::PACK_SHRINK);
    auto zoom_out_button = Gtk::make_managed<Gtk::Button>();
    zoom_out_button->set_image_from_icon_name("zoom-out-symbolic", Gtk::ICON_SIZE_BUTTON);
    zoom_out_button->signal_clicked().connect([this] { _engine.zoom_on_canva(-0.1); });
    _bottom_bar.pack_end(*zoom_out_button, Gtk::PACK_SHRINK);
}

void Menu::update_tool_types(std::vector<std::string> tool_types) {
    _tool_type_combo->remove_all();
    if (!tool_types.empty()) {
        for (const auto& option : tool_types) {
            _tool_type_combo->append(option);
        }
        _tool_type_combo->set_sensitive(true);
        _tool_type_combo->set_active(0);
        _tool_type_combo->signal_changed().connect(
            [this]() { _tool_type = _tool_type_combo->get_active_row_number(); });
    } else {
        _tool_type_combo->set_sensitive(false);
    }
}

} // namespace vipe