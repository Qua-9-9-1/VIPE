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
    auto label            = Gtk::make_managed<Gtk::Label>("Taille du pinceau:");
    auto brush_size_scale = Gtk::make_managed<Gtk::Scale>();
    auto horizontal_box   = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);

    brush_size_scale->set_range(1.0, 400.0);
    brush_size_scale->set_value(_tool_size);
    brush_size_scale->signal_value_changed().connect([this, brush_size_scale]() {
        _tool_size = static_cast<int>(brush_size_scale->get_value());
    });

    _tool_type_combo = Gtk::make_managed<Gtk::ComboBoxText>();
    horizontal_box->pack_start(*_tool_type_combo, Gtk::PACK_SHRINK);
    horizontal_box->pack_start(*brush_size_scale, Gtk::PACK_EXPAND_WIDGET);
    _sub_menu.set_orientation(Gtk::ORIENTATION_VERTICAL);
    _sub_menu.pack_start(*label, Gtk::PACK_SHRINK);
    _sub_menu.pack_start(*horizontal_box, Gtk::PACK_SHRINK);
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