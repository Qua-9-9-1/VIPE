#include "ColorPalette.hpp"

namespace vipe {
ColorPalette::ColorPalette()
    : _color_palette(40, 200), _first_color_box(nullptr), _second_color_box(nullptr),
      _chromatic_circle(nullptr), _is_dragging(false), _no_gauge_signal(false) {
    _first_color  = cv::Scalar(0, 0, 0, 255);
    _second_color = cv::Scalar(255, 255, 255, 255);

    Gtk::Box* main_box  = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    Gtk::Box* left_box  = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    Gtk::Box* right_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);

    build_selected_colors();
    build_chromatic_circle();
    build_colors_grid();
    build_hiddeable_box();
    build_colors_gauges();

    left_box->pack_start(_selected_colors_box, Gtk::PACK_SHRINK);
    left_box->pack_start(_chromatic_circle_box, Gtk::PACK_SHRINK);
    left_box->pack_start(_colors_grid_box, Gtk::PACK_SHRINK);

    right_box->pack_start(_hiddeable_box, Gtk::PACK_SHRINK);

    Gtk::ToggleButton* toggle_button = Gtk::make_managed<Gtk::ToggleButton>("Afficher les jauges");
    toggle_button->signal_toggled().connect([this, right_box, toggle_button] {
        if (toggle_button->get_active()) {
            right_box->show();
        } else {
            right_box->hide();
        }
    });

    _selected_colors_box.pack_start(*toggle_button, Gtk::PACK_SHRINK);

    main_box->pack_start(*left_box, Gtk::PACK_SHRINK);
    main_box->pack_start(*right_box, Gtk::PACK_SHRINK);

    _color_palette.set_panel_body(*main_box);
}

ColorPalette::~ColorPalette() {}

Gdk::RGBA ColorPalette::get_gtk_color(cv::Scalar color) {
    Gdk::RGBA rgba;

    if (color[3] < 0) {
        rgba.set_rgba(color[2] / 255.0, color[1] / 255.0, color[0] / 255.0, 1.0);
    } else {
        rgba.set_rgba(color[2] / 255.0, color[1] / 255.0, color[0] / 255.0, color[3] / 255.0);
    }
    return rgba;
}

cv::Scalar ColorPalette::get_cv_color(Gdk::RGBA color) {
    double r        = color.get_red();
    double g        = color.get_green();
    double b        = color.get_blue();
    double a        = color.get_alpha();
    auto   cv_color = cv::Scalar(b * 255, g * 255, r * 255, a * 255);

    return cv_color;
}

double ColorPalette::get_hue(cv::Scalar color) {
    double r     = color[2] / 255.0;
    double g     = color[1] / 255.0;
    double b     = color[0] / 255.0;
    double max   = std::max(r, std::max(g, b));
    double min   = std::min(r, std::min(g, b));
    double delta = max - min;
    double hue   = 0.0;

    if (delta == 0) {
        hue = 0;
    } else if (max == r) {
        hue = 60 * fmod(((g - b) / delta), 6);
    } else if (max == g) {
        hue = 60 * (((b - r) / delta) + 2);
    } else if (max == b) {
        hue = 60 * (((r - g) / delta) + 4);
    }
    return hue;
}

double ColorPalette::get_saturation(cv::Scalar color) {
    double r          = color[2] / 255.0;
    double g          = color[1] / 255.0;
    double b          = color[0] / 255.0;
    double max        = std::max(r, std::max(g, b));
    double min        = std::min(r, std::min(g, b));
    double delta      = max - min;
    double saturation = 0.0;

    if (max != 0) {
        saturation = delta / max;
    }
    return saturation;
}

double ColorPalette::get_value(cv::Scalar color) {
    double r   = color[2] / 255.0;
    double g   = color[1] / 255.0;
    double b   = color[0] / 255.0;
    double max = std::max(r, std::max(g, b));

    return max;
}

void ColorPalette::set_first_color(cv::Scalar color) {
    _first_color     = color;
    _no_gauge_signal = true;
    update_gauges_from_color();
    _first_color_box->override_background_color(get_gtk_color(_first_color));
}

void ColorPalette::set_hsv_first_color(double h, double s, double v) {
    v = v < 0 ? get_hue(_first_color) : v;
    s = s < 0 ? get_saturation(_first_color) : s;
    h = h < 0 ? get_value(_first_color) : h;

    hsv_to_rgb(h, s, v, _first_color[2], _first_color[1], _first_color[0]);
    set_first_color(_first_color);
}

void ColorPalette::set_second_color(cv::Scalar color) {
    _second_color = color;
    if (_second_color_box) {
        _second_color_box->override_background_color(get_gtk_color(_second_color));
    }
}

void ColorPalette::switch_colors() {
    cv::Scalar tmp_color = _second_color;
    set_second_color(_first_color);
    set_first_color(tmp_color);
}

void ColorPalette::build_selected_colors() {
    _selected_colors_box.set_orientation(Gtk::ORIENTATION_VERTICAL);

    Gtk::Box* colors_row_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);

    _first_color_box  = Gtk::make_managed<Gtk::EventBox>();
    _second_color_box = Gtk::make_managed<Gtk::EventBox>();
    _first_color_box->set_size_request(40, 40);
    _second_color_box->set_size_request(40, 40);

    _first_color_box->override_background_color(get_gtk_color(_first_color));
    _second_color_box->override_background_color(get_gtk_color(_second_color));

    Gtk::Button* switch_button = Gtk::make_managed<Gtk::Button>();
    switch_button->set_image_from_icon_name("object-flip-horizontal-symbolic",
                                            Gtk::ICON_SIZE_BUTTON);
    switch_button->set_relief(Gtk::RELIEF_NONE);

    switch_button->signal_clicked().connect(sigc::mem_fun(*this, &ColorPalette::switch_colors));

    colors_row_box->pack_start(*_first_color_box, Gtk::PACK_SHRINK);
    colors_row_box->pack_start(*switch_button, Gtk::PACK_SHRINK);
    colors_row_box->pack_start(*_second_color_box, Gtk::PACK_SHRINK);
    _selected_colors_box.pack_start(*colors_row_box, Gtk::PACK_SHRINK);
}

void ColorPalette::build_chromatic_circle() {
    _chromatic_circle_box.set_orientation(Gtk::ORIENTATION_VERTICAL);
    Gtk::Label* chromatic_circle_label = Gtk::make_managed<Gtk::Label>("Cercle chromatique");
    _chromatic_circle                  = Gtk::make_managed<Gtk::DrawingArea>();

    _chromatic_circle->set_size_request(100, 100);
    _chromatic_circle->signal_draw().connect(
        sigc::mem_fun(*this, &ColorPalette::on_draw_chromatic_circle));
    _chromatic_circle->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
                                  Gdk::POINTER_MOTION_MASK);

    _chromatic_circle->signal_draw().connect(
        sigc::mem_fun(*this, &ColorPalette::on_draw_chromatic_circle));
    _chromatic_circle->signal_button_press_event().connect(
        sigc::mem_fun(*this, &ColorPalette::on_chromatic_circle_click));
    _chromatic_circle->signal_button_release_event().connect(
        sigc::mem_fun(*this, &ColorPalette::on_chromatic_circle_release));
    _chromatic_circle->signal_motion_notify_event().connect(
        sigc::mem_fun(*this, &ColorPalette::on_chromatic_circle_motion));
    _chromatic_circle_box.pack_start(*chromatic_circle_label, Gtk::PACK_SHRINK);
    _chromatic_circle_box.pack_start(*_chromatic_circle, Gtk::PACK_SHRINK);
}

bool ColorPalette::on_draw_chromatic_circle(const Cairo::RefPtr<Cairo::Context>& cr) {
    double circle_dim = 100;
    double radius     = circle_dim / 2;
    double center     = radius;
    double _cursor_x  = -1;
    double _cursor_y  = -1;

    for (int y = 0; y < circle_dim; ++y) {
        for (int x = 0; x < circle_dim; ++x) {
            double dx       = x - center;
            double dy       = y - center;
            double distance = sqrt(dx * dx + dy * dy);

            if (distance <= radius) {
                double angle = atan2(dy, dx) * 180.0 / M_PI;
                if (angle < 0)
                    angle += 360.0;

                double hue        = angle;
                double saturation = distance / radius;
                double red, green, blue;
                hsv_to_rgb(hue, saturation, 1.0, red, green, blue);

                cr->set_source_rgb(red, green, blue);
                cr->rectangle(x, y, 1, 1);
                cr->fill();
            }
        }
    }
    double hue, saturation, value;
    rgb_to_hsv(_first_color[2] / 255.0, _first_color[1] / 255.0, _first_color[0] / 255.0, hue,
               saturation, value);
    double cursor_x = center + saturation * radius * cos(hue * M_PI / 180.0);
    double cursor_y = center + saturation * radius * sin(hue * M_PI / 180.0);
    cr->set_source_rgb(0.0, 0.0, 0.0);
    cr->arc(cursor_x, cursor_y, 5, 0, 2 * M_PI);
    cr->set_line_width(2);
    cr->stroke();
    cr->set_source_rgb(1.0, 1.0, 1.0);
    cr->arc(cursor_x, cursor_y, 3, 0, 2 * M_PI);
    cr->fill();
    return true;
}

bool ColorPalette::on_chromatic_circle_click(GdkEventButton* event) {
    _is_dragging = true;
    update_color_from_position(event->x, event->y);
    return true;
}

bool ColorPalette::on_chromatic_circle_release(GdkEventButton* event) {
    _is_dragging = false;
    return true;
}

bool ColorPalette::on_chromatic_circle_motion(GdkEventMotion* event) {
    if (_is_dragging) {
        update_color_from_position(event->x, event->y);
    }
    return true;
}

bool ColorPalette::update_color_from_position(int x, int y) {
    double circle_dim = 100;
    double radius     = circle_dim / 2;
    double center     = radius;

    double dx       = x - center;
    double dy       = y - center;
    double distance = sqrt(dx * dx + dy * dy);

    if (distance <= radius) {
        double angle = atan2(dy, dx) * 180.0 / M_PI;
        if (angle < 0)
            angle += 360.0;

        double hue        = angle;
        double saturation = distance / radius;

        double red, green, blue;
        hsv_to_rgb(hue, saturation, 1.0, red, green, blue);
        set_first_color(cv::Scalar(blue * 255, green * 255, red * 255, 1.0 * 255));

        return true;
    }

    return false;
}

void ColorPalette::rgb_to_hsv(double r, double g, double b, double& h, double& s, double& v) {
    double max = std::max({r, g, b});
    double min = std::min({r, g, b});
    v          = max;

    double delta = max - min;
    s            = (max == 0) ? 0 : delta / max;

    if (delta == 0) {
        h = 0; // Grayscale
    } else {
        if (r == max) {
            h = (g - b) / delta;
        } else if (g == max) {
            h = 2 + (b - r) / delta;
        } else {
            h = 4 + (r - g) / delta;
        }
        h *= 60;
        if (h < 0)
            h += 360;
    }
}

void ColorPalette::hsv_to_rgb(double h, double s, double v, double& r, double& g, double& b) {
    int    i = static_cast<int>(h / 60) % 6;
    double f = (h / 60) - i;
    double p = v * (1 - s);
    double q = v * (1 - f * s);
    double t = v * (1 - (1 - f) * s);

    switch (i) {
    case 0:
        r = v;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = v;
        b = p;
        break;
    case 2:
        r = p;
        g = v;
        b = t;
        break;
    case 3:
        r = p;
        g = q;
        b = v;
        break;
    case 4:
        r = t;
        g = p;
        b = v;
        break;
    case 5:
        r = v;
        g = p;
        b = q;
        break;
    }
}

void ColorPalette::build_colors_grid() {
    Gtk::Label*            colors_grid_label = Gtk::make_managed<Gtk::Label>("Palette de couleurs");
    Gtk::Grid*             colors_grid       = Gtk::make_managed<Gtk::Grid>();
    std::vector<Gdk::RGBA> colors            = {
        Gdk::RGBA("#FF0000"), Gdk::RGBA("#FF8800"), Gdk::RGBA("#FFFF00"), Gdk::RGBA("#00FF00"),
        Gdk::RGBA("#00FFFF"), Gdk::RGBA("#0000FF"), Gdk::RGBA("#8800FF"), Gdk::RGBA("#FF00FF"),
        Gdk::RGBA("#FF0088"), Gdk::RGBA("#884400"), Gdk::RGBA("#888888"), Gdk::RGBA("#000000"),
        Gdk::RGBA("#FFFFFF")};

    _colors_grid_box.set_orientation(Gtk::ORIENTATION_VERTICAL);
    colors_grid->set_column_homogeneous(true);
    colors_grid->set_row_homogeneous(true);
    for (int i = 0; i < 12; ++i) {
        Gtk::EventBox* color_box = Gtk::make_managed<Gtk::EventBox>();
        color_box->set_size_request(10, 10);
        color_box->override_background_color(colors[i]);
        Gdk::RGBA color = colors[i];
        color_box->signal_button_press_event().connect([this, color](GdkEventButton* event) {
            if (event->button == 1) {
                set_first_color(get_cv_color(color));
            } else if (event->button == 3) {
                set_second_color(get_cv_color(color));
            }
            return true;
        });
        colors_grid->attach(*color_box, i % colors.size(), i / colors.size());
    }
    _colors_grid_box.pack_start(*colors_grid_label, Gtk::PACK_SHRINK);
    _colors_grid_box.pack_start(*colors_grid, Gtk::PACK_SHRINK);
}

void ColorPalette::build_hiddeable_box() {
    Gtk::Label*    hiddeable_box_label = Gtk::make_managed<Gtk::Label>("Boîte cachable");
    Gtk::EventBox* hiddeable_event_box = Gtk::make_managed<Gtk::EventBox>();
    _hiddeable_box.set_orientation(Gtk::ORIENTATION_VERTICAL);
    _hiddeable_box.pack_start(*hiddeable_box_label, Gtk::PACK_SHRINK);
    _hiddeable_box.pack_start(*hiddeable_event_box, Gtk::PACK_SHRINK);
}

void ColorPalette::build_colors_gauges() {
    Gtk::Box*   colors_gauges       = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    Gtk::Label* colors_gauges_label = Gtk::make_managed<Gtk::Label>("Jauges de couleurs");

    _red_gauge        = Gtk::make_managed<Gtk::Scale>();
    _green_gauge      = Gtk::make_managed<Gtk::Scale>();
    _blue_gauge       = Gtk::make_managed<Gtk::Scale>();
    _hue_gauge        = Gtk::make_managed<Gtk::Scale>();
    _saturation_gauge = Gtk::make_managed<Gtk::Scale>();
    _value_gauge      = Gtk::make_managed<Gtk::Scale>();
    _alpha_gauge      = Gtk::make_managed<Gtk::Scale>();

    _red_gauge->set_range(0, 255);
    _red_gauge->set_digits(0);
    _green_gauge->set_range(0, 255);
    _green_gauge->set_digits(0);
    _blue_gauge->set_range(0, 255);
    _blue_gauge->set_digits(0);
    _alpha_gauge->set_range(0, 255);
    _alpha_gauge->set_digits(0);
    _hue_gauge->set_range(0, 359);
    _hue_gauge->set_digits(0);
    _saturation_gauge->set_range(0, 100);
    _saturation_gauge->set_digits(0);
    _value_gauge->set_range(0, 100);
    _value_gauge->set_digits(0);

    _red_gauge->signal_value_changed().connect([this]() { update_RGBA_color(0); });
    _green_gauge->signal_value_changed().connect([this]() { update_RGBA_color(1); });
    _blue_gauge->signal_value_changed().connect([this]() { update_RGBA_color(2); });
    _hue_gauge->signal_value_changed().connect([this]() { update_HSV_color(0); });
    _saturation_gauge->signal_value_changed().connect([this]() { update_HSV_color(1); });
    _value_gauge->signal_value_changed().connect([this]() { update_HSV_color(2); });
    _alpha_gauge->signal_value_changed().connect([this]() { update_RGBA_color(3); });

    update_gauges_from_color();

    colors_gauges->pack_start(*colors_gauges_label, Gtk::PACK_SHRINK);
    colors_gauges->pack_start(*_red_gauge, Gtk::PACK_SHRINK);
    colors_gauges->pack_start(*_green_gauge, Gtk::PACK_SHRINK);
    colors_gauges->pack_start(*_blue_gauge, Gtk::PACK_SHRINK);
    colors_gauges->pack_start(*_hue_gauge, Gtk::PACK_SHRINK);
    colors_gauges->pack_start(*_saturation_gauge, Gtk::PACK_SHRINK);
    colors_gauges->pack_start(*_value_gauge, Gtk::PACK_SHRINK);
    colors_gauges->pack_start(*_alpha_gauge, Gtk::PACK_SHRINK);

    _hiddeable_box.pack_start(*colors_gauges, Gtk::PACK_SHRINK);
}

void ColorPalette::update_gauges_from_color() {
    _red_gauge->set_value(_first_color[2]);
    _green_gauge->set_value(_first_color[1]);
    _blue_gauge->set_value(_first_color[0]);
    _alpha_gauge->set_value(_first_color[3]);

    double hue, saturation, value;
    rgb_to_hsv(_first_color[2] / 255.0, _first_color[1] / 255.0, _first_color[0] / 255.0, hue,
               saturation, value);

    _hue_gauge->set_value(hue);
    _saturation_gauge->set_value(saturation * 100);
    _value_gauge->set_value(value * 100);
    _no_gauge_signal = true;
}

void ColorPalette::update_RGBA_color(int color) {
    switch (color) {
    case 0:
        _first_color[2] = _red_gauge->get_value();
        break;
    case 1:
        _first_color[1] = _green_gauge->get_value();
        break;
    case 2:
        _first_color[0] = _blue_gauge->get_value();
        break;
    case 3:
        _first_color[3] = _alpha_gauge->get_value();
        break;
    default:
        break;
    }
    set_first_color(_first_color);
    _no_gauge_signal = false;
    _hiddeable_box.queue_draw();
}

void ColorPalette::update_HSV_color(int gauge) {

    double hue        = _hue_gauge->get_value();
    double saturation = _saturation_gauge->get_value() / 100.0;
    double value      = _value_gauge->get_value() / 100.0;

    double red, green, blue;
    hsv_to_rgb(hue, saturation, value, red, green, blue);

    set_first_color(cv::Scalar(blue * 255, green * 255, red * 255, _first_color[3]));

    _red_gauge->set_value(_first_color[2]);
    _green_gauge->set_value(_first_color[1]);
    _blue_gauge->set_value(_first_color[0]);

    _hiddeable_box.queue_draw();
}

} // namespace vipe
