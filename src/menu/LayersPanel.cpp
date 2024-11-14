#include "LayersPanel.hpp"

namespace vipe {
LayersPanel::LayersPanel(Canva& canva)
    : _canva(canva), _layer_panel(300, 300), _layer_options(100, 100) {
    build_layer_panel();
    build_options_panel();
}

LayersPanel::~LayersPanel() {}

void LayersPanel::build_layer_panel() {
    _layer_list_container.set_orientation(Gtk::ORIENTATION_VERTICAL);
    _layer_panel.set_panel_body(_layer_list_container);
    update_layer_list();
}

void LayersPanel::update_layer_list() {
    auto layers = _canva.get_layers();

    clear_layer_list();
    for (int i = 0; i < layers.size(); ++i) {
        add_layer_to_list(i);
    }
    add_add_layer_button();
    _layer_list_container.show_all();
}

void LayersPanel::clear_layer_list() {
    _layer_list_container.foreach (
        [this](Gtk::Widget& child) { _layer_list_container.remove(child); });
}

void LayersPanel::add_layer_to_list(int i) {
    auto event_box = Gtk::make_managed<Gtk::EventBox>();
    auto inner_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    auto label     = Gtk::make_managed<Gtk::Label>("Calque " + std::to_string(i + 1));

    event_box->add(*label);
    event_box->set_size_request(120, 50);
    label->set_ellipsize(Pango::ELLIPSIZE_END);
    label->set_max_width_chars(15);
    event_box->signal_button_press_event().connect([this, i](GdkEventButton* event) {
        if (event->button == 1) {
            on_layer_clicked(i);
            return true;
        } else if (event->button == 3) {
            std::cout << "drag and drop " << i << std::endl;
            return true;
        }
        return false;
    });
    inner_box->pack_start(*event_box, Gtk::PACK_EXPAND_WIDGET);
    add_delete_button(inner_box, i);
    add_options_button(inner_box, i);
    add_visibility_button(inner_box, i);
    _layer_list_container.pack_start(*inner_box, Gtk::PACK_SHRINK);
}

void LayersPanel::add_visibility_button(Gtk::Box* layer_box, int i) {
    auto visibility_button = Gtk::make_managed<Gtk::Button>();
    visibility_button->set_image_from_icon_name("view-visible-symbolic", Gtk::ICON_SIZE_BUTTON);
    visibility_button->signal_clicked().connect([this, i, visibility_button] {
        toggle_layer_visibility(i);
        bool is_visible = _canva.is_layer_visible(i);
        visibility_button->set_image_from_icon_name(
            is_visible ? "view-visible-symbolic" : "view-hidden-symbolic", Gtk::ICON_SIZE_BUTTON);
    });
    layer_box->pack_end(*visibility_button, Gtk::PACK_SHRINK);
}

void LayersPanel::add_options_button(Gtk::Box* layer_box, int i) {
    auto options_button = Gtk::make_managed<Gtk::Button>();
    options_button->set_image_from_icon_name("emblem-system-symbolic", Gtk::ICON_SIZE_BUTTON);
    options_button->signal_clicked().connect([this, i] { _layer_options.display_panel(); });
    layer_box->pack_end(*options_button, Gtk::PACK_SHRINK);
}

void LayersPanel::add_delete_button(Gtk::Box* layer_box, int i) {
    auto delete_button = Gtk::make_managed<Gtk::Button>();
    delete_button->set_image_from_icon_name("window-close-symbolic", Gtk::ICON_SIZE_BUTTON);
    delete_button->signal_clicked().connect([this, i] { on_delete_layer(i); });
    layer_box->pack_end(*delete_button, Gtk::PACK_SHRINK);
}

void LayersPanel::add_add_layer_button() {
    Gtk::Button* add_layer_button = Gtk::manage(new Gtk::Button());
    add_layer_button->set_image_from_icon_name("list-add-symbolic", Gtk::ICON_SIZE_BUTTON);
    add_layer_button->signal_clicked().connect([this] { on_add_layer(); });
    _layer_list_container.pack_end(*add_layer_button, Gtk::PACK_SHRINK);
}

void LayersPanel::on_layer_clicked(int index) {
    _canva.set_selected_layer(index);
    update_options_panel();
}

bool LayersPanel::on_layer_drag_motion(int x, int y) { return true; }

void LayersPanel::on_add_layer() {
    _canva.add_layer();
    update_layer_list();
}

void LayersPanel::on_delete_layer(int index) {
    _canva.delete_layer(index);
    update_layer_list();
}

void LayersPanel::toggle_layer_visibility(int index) {
    auto& layers          = _canva.get_layers();
    layers[index].visible = !layers[index].visible;
}

void LayersPanel::build_options_panel() {
    _layer_options_container.set_orientation(Gtk::ORIENTATION_VERTICAL);
    _layer_options.set_panel_body(_layer_options_container);
    update_options_panel();
    _layer_options.hide_panel();
}

void LayersPanel::update_options_panel() {
    auto header_box    = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    auto layer_label   = Gtk::make_managed<Gtk::Label>("CALQUE");
    auto opacity_scale = Gtk::make_managed<Gtk::Scale>(Gtk::ORIENTATION_HORIZONTAL);
    auto button_box    = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    auto current_layer = _canva.get_selected_layer();
    int  opacity       = current_layer.opacity;

    _layer_options.clear_panel_body();
    header_box->pack_start(*layer_label, Gtk::PACK_EXPAND_WIDGET);
    _layer_options_container.pack_start(*header_box, Gtk::PACK_SHRINK);
    opacity_scale->set_range(0, 100);
    opacity_scale->set_value(opacity);
    opacity_scale->set_digits(0);
    _layer_options_container.pack_start(*opacity_scale, Gtk::PACK_EXPAND_WIDGET);

    auto validate_button = Gtk::make_managed<Gtk::Button>("Valider");
    validate_button->signal_clicked().connect([this, opacity_scale, opacity] {
        on_validate_options_panel(static_cast<int>(opacity_scale->get_value()));
    });
    button_box->pack_start(*validate_button, Gtk::PACK_EXPAND_WIDGET);

    auto close_button = Gtk::make_managed<Gtk::Button>("Annuler");
    close_button->signal_clicked().connect([this] { on_close_options_panel(); });
    button_box->pack_start(*close_button, Gtk::PACK_EXPAND_WIDGET);

    _layer_options_container.pack_start(*button_box, Gtk::PACK_SHRINK);
    _layer_options.hide_panel();
}

void LayersPanel::on_validate_options_panel(int opacity) {
    auto& current_layer   = _canva.get_selected_layer();
    current_layer.opacity = opacity;
    on_close_options_panel();
}

void LayersPanel::on_close_options_panel() { _layer_options.hide_panel(); }
} // namespace vipe