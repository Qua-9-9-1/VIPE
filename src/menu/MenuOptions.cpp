#include "Menu.hpp"
#include "Engine.hpp"

namespace vipe {
    void Menu::create_file_option()
    {
        auto file_menu = Gtk::make_managed<Gtk::Menu>();
        auto file_item = Gtk::make_managed<Gtk::MenuItem>("Fichier");
        file_item->set_submenu(*file_menu);

        auto new_item = Gtk::make_managed<Gtk::MenuItem>("Nouveau");
        new_item->signal_activate().connect(sigc::mem_fun(_window, &MyWindow::new_file));
        file_menu->append(*new_item);

        auto open_item = Gtk::make_managed<Gtk::MenuItem>("Ouvrir");
        open_item->signal_activate().connect(sigc::mem_fun(_window, &MyWindow::open_file));
        file_menu->append(*open_item);

        auto save_item = Gtk::make_managed<Gtk::MenuItem>("Enregistrer");
        save_item->signal_activate().connect(sigc::mem_fun(_window, &MyWindow::save_file));
        file_menu->append(*save_item);

        auto save_as_item = Gtk::make_managed<Gtk::MenuItem>("Enregistrer sous");
        save_as_item->signal_activate().connect(sigc::mem_fun(_window, &MyWindow::save_as_file));
        file_menu->append(*save_as_item);

        auto close_item = Gtk::make_managed<Gtk::MenuItem>("Fermer");
        close_item->signal_activate().connect(sigc::mem_fun(_window, &MyWindow::close_file));
        file_menu->append(*close_item);

        _menu_bar.append(*file_item);
    }

    void Menu::create_display_option()
    {
        auto display_menu = Gtk::make_managed<Gtk::Menu>();
        auto display_item = Gtk::make_managed<Gtk::MenuItem>("Affichage");
        display_item->set_submenu(*display_menu);

        auto fullscreen_item = Gtk::make_managed<Gtk::MenuItem>("Plein écran");
        fullscreen_item->signal_activate().connect(sigc::mem_fun(_window, &MyWindow::fullscreen));
        display_menu->append(*fullscreen_item);

        _menu_bar.append(*display_item);
    }

    void Menu::create_image_option()
    {
        auto image_menu = Gtk::make_managed<Gtk::Menu>();
        auto image_item = Gtk::make_managed<Gtk::MenuItem>("Image");
        image_item->set_submenu(*image_menu);

        auto resize_item = Gtk::make_managed<Gtk::MenuItem>("Redimensionner");
        resize_item->signal_activate().connect(sigc::mem_fun(_window, &MyWindow::resize_image));
        image_menu->append(*resize_item);

        _menu_bar.append(*image_item);
    }

    void Menu::create_effect_option()
    {
        auto effect_menu = Gtk::make_managed<Gtk::Menu>();
        auto effect_item = Gtk::make_managed<Gtk::MenuItem>("Effet");
        effect_item->set_submenu(*effect_menu);

        auto blur_item = Gtk::make_managed<Gtk::MenuItem>("Flou");
        blur_item->signal_activate().connect(sigc::mem_fun(_window, &MyWindow::blur_effect));
        effect_menu->append(*blur_item);
    
        _menu_bar.append(*effect_item);
    }
}