#include "LayersPanel.hpp"

namespace vipe {
LayersPanel::LayersPanel(Canva& canva, Gtk::DrawingArea& drawing_area)
    : _canva(canva), _drawing_area(drawing_area), _layer_panel(300, 300), _layer_options(100, 100),
      _last_selected_layer(nullptr), _opacity_scale(nullptr), _blend_mode_combo(nullptr) {
    build_layer_panel();
    build_options_panel();
}

LayersPanel::~LayersPanel() {}

void LayersPanel::build_layer_panel() {
    _layer_list_container.set_orientation(Gtk::ORIENTATION_VERTICAL);
    _layer_panel.set_panel_body(_layer_list_container);
    update_layer_list();
}

void LayersPanel::set_canva(Canva& canva) {
    _canva = canva;
    build_layer_panel();
    build_options_panel();
    // on_close_options_panel();
    // update_layer_list();
    // update_options_panel();
}

void LayersPanel::update_layer_list() {
    auto layers = _canva.get_layers();

    clear_layer_list();
    _layers.clear();
    for (int i = 0; i < layers.size(); ++i) {
        add_layer_to_list(i);
    }
    add_add_layer_button();
    add_layer_up_button(_canva.get_selected_layer_index());
    add_layer_down_button(_canva.get_selected_layer_index());
    add_image_layer_button();
    _layer_list_container.show_all();
}

void LayersPanel::clear_layer_list() {
    _layer_list_container.foreach (
        [this](Gtk::Widget& child) { _layer_list_container.remove(child); });
}

void LayersPanel::add_layer_to_list(int i) {
    auto event_box = Gtk::make_managed<Gtk::EventBox>();
    auto inner_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    auto label     = Gtk::make_managed<Gtk::Label>(_canva.get_layers()[i].name);

    event_box->add(*label);
    event_box->set_size_request(120, 50);
    label->set_ellipsize(Pango::ELLIPSIZE_END);
    label->set_max_width_chars(15);
    event_box->signal_button_press_event().connect([this, i, inner_box](GdkEventButton* event) {
        if (event->button == 1) {
            on_layer_clicked(i, inner_box);
            return true;
        } else if (event->button == 3) {
            std::cout << "drag and drop " << i << std::endl;
            return true;
        }
        return false;
    });
    inner_box->override_background_color(Gdk::RGBA("#E9E9F4"));
    inner_box->pack_start(*event_box, Gtk::PACK_EXPAND_WIDGET);
    add_delete_button(inner_box, i);
    add_options_button(inner_box, i);
    add_visibility_button(inner_box, i);
    _layers.push_back(inner_box);
    if (_canva.get_selected_layer_index() == i) {
        _last_selected_layer = inner_box;
        inner_box->override_background_color(Gdk::RGBA("#DFDFE5"));
    }
    _layer_list_container.pack_start(*inner_box, Gtk::PACK_SHRINK);
}

void LayersPanel::add_visibility_button(Gtk::Box* layer_box, int i) {
    auto visibility_button = Gtk::make_managed<Gtk::Button>();
    visibility_button->set_image_from_icon_name("view-visible-symbolic", Gtk::ICON_SIZE_BUTTON);
    visibility_button->signal_clicked().connect(
        [this, i, visibility_button] { toggle_layer_visibility(i, visibility_button); });
    layer_box->pack_end(*visibility_button, Gtk::PACK_SHRINK);
}

void LayersPanel::add_options_button(Gtk::Box* layer_box, int i) {
    auto options_button = Gtk::make_managed<Gtk::Button>();
    options_button->set_image_from_icon_name("emblem-system-symbolic", Gtk::ICON_SIZE_BUTTON);
    options_button->signal_clicked().connect([this] { set_layer_options_panel(); });
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

void LayersPanel::add_layer_up_button(int index) {
    Gtk::Button* _move_layer_up_button = Gtk::manage(new Gtk::Button());
    _move_layer_up_button->set_image_from_icon_name("go-up-symbolic", Gtk::ICON_SIZE_BUTTON);
    if (index == 0) {
        _move_layer_up_button->set_sensitive(false);
    }
    _move_layer_up_button->signal_clicked().connect([this, index] { on_move_layer_up(index); });
    _layer_list_container.pack_end(*_move_layer_up_button, Gtk::PACK_SHRINK);
}

void LayersPanel::add_layer_down_button(int index) {
    Gtk::Button* _move_layer_down_button = Gtk::manage(new Gtk::Button());
    _move_layer_down_button->set_image_from_icon_name("go-down-symbolic", Gtk::ICON_SIZE_BUTTON);
    if (index == _canva.get_layers().size() - 1) {
        _move_layer_down_button->set_sensitive(false);
    }
    _move_layer_down_button->signal_clicked().connect([this, index] { on_move_layer_down(index); });
    _layer_list_container.pack_end(*_move_layer_down_button, Gtk::PACK_SHRINK);
}

void LayersPanel::add_image_layer_button() {
    Gtk::Button* add_image_layer_button = Gtk::manage(new Gtk::Button());
    add_image_layer_button->set_image_from_icon_name("image-x-generic-symbolic",
                                                     Gtk::ICON_SIZE_BUTTON);
    add_image_layer_button->signal_clicked().connect([this] { on_add_layer_from_image(); });
    _layer_list_container.pack_end(*add_image_layer_button, Gtk::PACK_SHRINK);
}

void LayersPanel::on_layer_clicked(int index, Gtk::Box* clicked_layer) {
    _last_selected_layer->override_background_color(Gdk::RGBA("#E9E9F4"));
    clicked_layer->override_background_color(Gdk::RGBA("#DFDFE5"));
    _canva.set_selected_layer(index);
    _blend_mode_combo->set_active(_canva.get_selected_layer().blend_mode);
    _last_selected_layer = clicked_layer;
    on_close_options_panel();
}

bool LayersPanel::on_layer_drag_motion(int x, int y) { return true; }

void LayersPanel::on_add_layer() {
    _canva.add_layer();
    update_layer_list();
}

void LayersPanel::on_move_layer_up(int index) {
    _canva.move_layer_up(index);
    update_layer_list();
    _drawing_area.queue_draw();
}

void LayersPanel::on_move_layer_down(int index) {
    _canva.move_layer_down(index);
    update_layer_list();
    _drawing_area.queue_draw();
}

void LayersPanel::on_add_layer_from_image() {
    Gtk::FileChooserDialog dialog("Choisir une image", Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.add_button("Annuler", Gtk::RESPONSE_CANCEL);
    dialog.add_button("Ouvrir", Gtk::RESPONSE_OK);
    auto filter_image = Gtk::FileFilter::create();
    filter_image->set_name("Images");
    filter_image->add_mime_type("image/jpeg");
    filter_image->add_mime_type("image/png");
    dialog.add_filter(filter_image);
    int result = dialog.run();
    if (result == Gtk::RESPONSE_OK) {
        std::string filename = dialog.get_filename();
        _canva.add_layer_from_image(filename);
    }
    update_layer_list();
    _drawing_area.queue_draw();
}

void LayersPanel::toggle_layer_visibility(int index, Gtk::Button* visibility_button) {
    auto& layers     = _canva.get_layers();
    bool  is_visible = _canva.is_layer_visible(index);

    layers[index].visible = !layers[index].visible;
    visibility_button->set_image_from_icon_name(
        is_visible ? "view-visible-symbolic" : "view-hidden-symbolic", Gtk::ICON_SIZE_BUTTON);
    _drawing_area.queue_draw();
}

void LayersPanel::set_layer_options_panel() {
    _opacity_scale->set_value(_canva.get_selected_layer().opacity);
    _layer_options.display_panel();
}

void LayersPanel::on_delete_layer(int index) {
    if (_canva.get_layers().size() == 1) {
        return;
    }
    _canva.delete_layer(index);
    if (index >= 0) {
        _canva.set_selected_layer(index - 1);
    }
    update_layer_list();
    on_close_options_panel();
    _drawing_area.queue_draw();
}

void LayersPanel::build_options_panel() {
    _layer_options_container.set_orientation(Gtk::ORIENTATION_VERTICAL);
    _layer_options.set_panel_body(_layer_options_container);
    update_options_panel();
}

void LayersPanel::update_options_panel() {
    auto header_box   = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    auto layer_label  = Gtk::make_managed<Gtk::Label>(_canva.get_selected_layer().name);
    auto button_box   = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    _opacity_scale    = Gtk::make_managed<Gtk::Scale>(Gtk::ORIENTATION_HORIZONTAL);
    _blend_mode_combo = Gtk::make_managed<Gtk::ComboBoxText>();

    _layer_options.clear_panel_body();
    header_box->pack_start(*layer_label, Gtk::PACK_EXPAND_WIDGET);
    _layer_options_container.pack_start(*header_box, Gtk::PACK_SHRINK);
    _opacity_scale->set_range(0, 100);
    _opacity_scale->set_value(_canva.get_selected_layer().opacity);
    _opacity_scale->set_digits(0);
    _layer_options_container.pack_start(*_opacity_scale, Gtk::PACK_EXPAND_WIDGET);

    _blend_mode_combo->append("Normal");
    _blend_mode_combo->append("Add");
    _blend_mode_combo->append("Multiply");
    _blend_mode_combo->append("XOR");

    _layer_options_container.pack_start(*_blend_mode_combo, Gtk::PACK_EXPAND_WIDGET);
    auto validate_button = Gtk::make_managed<Gtk::Button>("Valider");
    validate_button->signal_clicked().connect(
        [this] { on_validate_options_panel(static_cast<int>(_opacity_scale->get_value())); });
    _blend_mode_combo->signal_changed().connect(
        [this]() { set_layer_mode(_blend_mode_combo->get_active_row_number()); });
    _blend_mode_combo->set_active(_canva.get_selected_layer().blend_mode);

    button_box->pack_start(*validate_button, Gtk::PACK_EXPAND_WIDGET);
    auto close_button = Gtk::make_managed<Gtk::Button>("Annuler");
    close_button->signal_clicked().connect([this] { on_close_options_panel(); });
    button_box->pack_start(*close_button, Gtk::PACK_EXPAND_WIDGET);

    _layer_options_container.pack_start(*button_box, Gtk::PACK_SHRINK);
}

void LayersPanel::on_validate_options_panel(int opacity) {
    auto& current_layer   = _canva.get_selected_layer();
    current_layer.opacity = opacity;
    on_close_options_panel();
}

void LayersPanel::on_close_options_panel() { _layer_options.hide_panel(); }

void LayersPanel::set_layer_mode(int mode) {
    auto& current_layer      = _canva.get_selected_layer();
    current_layer.blend_mode = mode;
    _drawing_area.queue_draw();
}
} // namespace vipe