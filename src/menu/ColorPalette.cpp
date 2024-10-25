#include "ColorPalette.hpp"

namespace vipe {
    ColorPalette::ColorPalette(): _color_palette(10, 10)
    {
        set_first_color(cv::Scalar(0, 0, 0, 255));
        set_second_color(cv::Scalar(255, 255, 255, 255));
    }

    ColorPalette::~ColorPalette() {}
}