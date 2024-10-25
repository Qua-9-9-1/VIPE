#include "LayersPanel.hpp"

namespace vipe {
    LayersPanel::LayersPanel(Canva& canva): _canva(canva), _layer_panel(300, 300)
    {
        build_layer_panel();
    }

    LayersPanel::~LayersPanel() {}

    void LayersPanel::build_layer_panel()
    {
        _layer_list_container.set_orientation(Gtk::ORIENTATION_VERTICAL);
        _layer_panel.set_panel_body(_layer_list_container);
        update_layer_list();   
    }

    void LayersPanel::update_layer_list()
    {
        auto layers = _canva.get_layers();

        clear_layer_list();

        for (int i = 0; i < layers.size(); ++i) {
            add_layer_to_list(i);
        }

        add_add_layer_button();
        _layer_list_container.show_all();
    }

    void LayersPanel::clear_layer_list() {
        _layer_list_container.foreach([this](Gtk::Widget& child) {
            _layer_list_container.remove(child);
        });
    }

    void LayersPanel::add_layer_to_list(int i)
    {
        auto layer_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
        auto label = Gtk::make_managed<Gtk::Label>("Calque " + std::to_string(i + 1));

        layer_box->set_size_request(200, 50);
        layer_box->pack_start(*label, Gtk::PACK_SHRINK);

        auto event_box = create_event_box(i, layer_box);
        add_delete_button(layer_box, i);
        add_visibility_button(layer_box, i);

        _layer_list_container.pack_start(*event_box, Gtk::PACK_SHRINK);
    }

    Gtk::EventBox* LayersPanel::create_event_box(int i, Gtk::Box* layer_box)
    {
        auto event_box = Gtk::make_managed<Gtk::EventBox>();
        event_box->add(*layer_box);
        event_box->set_events(Gdk::BUTTON_PRESS_MASK);
        event_box->signal_button_press_event().connect([this, i](GdkEventButton* event) {
            on_layer_clicked(i);
            return true;
        });
        return event_box;
    }

    void LayersPanel::add_delete_button(Gtk::Box* layer_box, int i)
    {
        Gtk::Button* delete_button = Gtk::manage(new Gtk::Button());
        delete_button->set_image_from_icon_name("window-close-symbolic", Gtk::ICON_SIZE_BUTTON);
        delete_button->signal_clicked().connect([this, i] {
            on_delete_layer(i);
        });
        layer_box->pack_end(*delete_button, Gtk::PACK_SHRINK);
    }

    void LayersPanel::add_visibility_button(Gtk::Box* layer_box, int i)
    {
        Gtk::Button* visibility_button = Gtk::manage(new Gtk::Button());
        visibility_button->set_image_from_icon_name("view-visible-symbolic", Gtk::ICON_SIZE_BUTTON);
        visibility_button->signal_clicked().connect([this, i, visibility_button] {
            toggle_layer_visibility(i);
            bool is_visible = _canva.is_layer_visible(i);
            visibility_button->set_image_from_icon_name(is_visible ? "view-visible-symbolic" : "view-hidden-symbolic", Gtk::ICON_SIZE_BUTTON);
        });
        layer_box->pack_end(*visibility_button, Gtk::PACK_SHRINK);
    }

    void LayersPanel::add_add_layer_button()
    {
        Gtk::Button* add_layer_button = Gtk::manage(new Gtk::Button());
        add_layer_button->set_image_from_icon_name("list-add-symbolic", Gtk::ICON_SIZE_BUTTON);
        add_layer_button->signal_clicked().connect([this] {
            on_add_layer();
        });
        _layer_list_container.pack_end(*add_layer_button, Gtk::PACK_SHRINK);
    }

    void LayersPanel::on_layer_clicked(int index)
    {
        std::cout << "Calque sélectionné : " << index << std::endl;
        _canva.set_selected_layer(index);
    }

    bool LayersPanel::on_layer_drag_motion(int x, int y)
    {
        return true;
    }

    void LayersPanel::on_add_layer()
    {
        _canva.add_layer();
        update_layer_list();
    }

    void LayersPanel::on_delete_layer(int index)
    {
        _canva.delete_layer(index);
        update_layer_list();
    }

    void LayersPanel::toggle_layer_visibility(int index)
    {
        auto& layers = _canva.get_layers();
        layers[index].visible = !layers[index].visible;
    }
}