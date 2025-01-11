#include "Canva.hpp"

namespace vipe {

void Canva::update_layer_cache() {
    if (_cached_layers.size() != _layers.size()) {
        _cached_layers.resize(_layers.size());
    }

    for (size_t i = 0; i < _layers.size(); ++i) {
        const auto& layer        = _layers[i];
        auto&       cached_layer = _cached_layers[i];

        if (layer.visible != cached_layer.visible || layer.blend_mode != cached_layer.blend_mode ||
            layer.opacity != cached_layer.opacity || _zoom_factor != cached_layer.zoom_factor ||
            cached_layer.needs_update) {
            convert_to_RGBA(layer.image, cached_layer.cached_image);
            cv::resize(cached_layer.cached_image, cached_layer.cached_image, cv::Size(),
                       _zoom_factor, _zoom_factor, cv::INTER_NEAREST);
            cached_layer.visible      = layer.visible;
            cached_layer.blend_mode   = layer.blend_mode;
            cached_layer.opacity      = layer.opacity;
            cached_layer.zoom_factor  = _zoom_factor;
            cached_layer.needs_update = false;
        }
    }
}

void Canva::add_layer() {
    Layer layer;
    layer.image      = cv::Mat(_image.size(), _image.type(), cv::Scalar(0, 0, 0, 0));
    layer.visible    = true;
    layer.opacity    = 100;
    layer.name       = "Calque " + std::to_string(_layers.size() + 1);
    layer.blend_mode = 0;
    _selected_layer  = _layers.size();
    _layers.push_back(layer);
}

void Canva::add_layer_from_image(const std::string& filename) {
    Layer layer;
    layer.image = cv::imread(filename, cv::IMREAD_UNCHANGED);
    if (layer.image.empty()) {
        std::cerr << "Erreur lors du chargement de l'image." << std::endl;
        return;
    }
    convert_to_RGBA(layer.image, layer.image);
    int new_width  = std::max(layer.image.cols, _image.cols);
    int new_height = std::max(layer.image.rows, _image.rows);

    if (new_width > _image.cols || new_height > _image.rows) {
        cv::Mat new_canva(cv::Size(new_width, new_height), _image.type(), cv::Scalar(0, 0, 0, 0));
        _image.copyTo(new_canva(cv::Rect(0, 0, _image.cols, _image.rows)));
        _image = new_canva;
        for (auto& existing_layer : _layers) {
            cv::Mat resized_layer(cv::Size(new_width, new_height), existing_layer.image.type(),
                                  cv::Scalar(0, 0, 0, 0));
            existing_layer.image.copyTo(resized_layer(
                cv::Rect(0, 0, existing_layer.image.cols, existing_layer.image.rows)));
            existing_layer.image = resized_layer;
        }
    }
    layer.name       = "Calque " + std::to_string(_layers.size() + 1);
    layer.visible    = true;
    layer.opacity    = 100;
    layer.blend_mode = 0;
    _selected_layer  = _layers.size();
    _layers.push_back(layer);
    update_background();
}

void Canva::delete_layer(int index) {
    if (index < 0 || index >= _layers.size()) {
        throw std::out_of_range("Couche hors limites");
    }
    _layers.erase(_layers.begin() + index);
}

void Canva::move_layer(int index, int new_index) {
    if (index < 0 || index >= _layers.size() || new_index < 0 || new_index >= _layers.size()) {
        throw std::out_of_range("Couche hors limites");
    }
    auto layer_to_move = _layers[index];
    _layers.erase(_layers.begin() + index);
    _layers.insert(_layers.begin() + new_index, layer_to_move);
    _selected_layer = new_index;
}

void Canva::move_layer_up(int index) {
    if (index < 0 || index >= _layers.size()) {
        throw std::out_of_range("Couche hors limites");
    }
    if (index == 0)
        return;
    auto layer_to_move = _layers[index];
    _layers.erase(_layers.begin() + index);
    _layers.insert(_layers.begin() + index - 1, layer_to_move);
    _selected_layer = index - 1;
}

void Canva::move_layer_down(int index) {
    if (index < 0 || index >= _layers.size()) {
        throw std::out_of_range("Couche hors limites");
    }
    if (index == _layers.size() - 1)
        return;
    auto layer_to_move = _layers[index];
    _layers.erase(_layers.begin() + index);
    _layers.insert(_layers.begin() + index + 1, layer_to_move);
    _selected_layer = index + 1;
}

void Canva::merge_layers() { _image = get_merged_image(); }

cv::Mat Canva::blend_normal(const cv::Mat& base, const cv::Mat& overlay, double alpha) {
    CV_Assert(base.type() == CV_8UC4 && overlay.type() == CV_8UC4);
    cv::Mat result = base.clone();
    for (int y = 0; y < base.rows; ++y) {
        for (int x = 0; x < base.cols; ++x) {
            cv::Vec4b bg_pixel = base.at<cv::Vec4b>(y, x);
            cv::Vec4b fg_pixel = overlay.at<cv::Vec4b>(y, x);

            double fg_alpha = (fg_pixel[3] / 255.0) * alpha;
            double bg_alpha = bg_pixel[3] / 255.0;

            double out_alpha = fg_alpha + bg_alpha * (1.0 - fg_alpha);

            if (out_alpha > 0) {
                for (int c = 0; c < 3; ++c) {
                    result.at<cv::Vec4b>(y, x)[c] = static_cast<uchar>(
                        (fg_pixel[c] * fg_alpha + bg_pixel[c] * bg_alpha * (1.0 - fg_alpha)) /
                        out_alpha);
                }
                result.at<cv::Vec4b>(y, x)[3] = static_cast<uchar>(out_alpha * 255);
            }
        }
    }
    return result;
}

cv::Mat blend_multiply(const cv::Mat& base, const cv::Mat& overlay) {
    CV_Assert(base.type() == CV_8UC4 && overlay.type() == CV_8UC4);
    cv::Mat result = base.clone();
    for (int y = 0; y < base.rows; ++y) {
        for (int x = 0; x < base.cols; ++x) {
            cv::Vec4b bg_pixel = base.at<cv::Vec4b>(y, x);
            cv::Vec4b fg_pixel = overlay.at<cv::Vec4b>(y, x);

            for (int c = 0; c < 3; ++c) {
                result.at<cv::Vec4b>(y, x)[c] =
                    static_cast<uchar>((bg_pixel[c] / 255.0) * (fg_pixel[c] / 255.0) * 255);
            }
            result.at<cv::Vec4b>(y, x)[3] = bg_pixel[3];
        }
    }
    return result;
}

void Canva::mark_layer_for_update(int layer_index) {
    if (layer_index < _cached_layers.size()) {
        _cached_layers[layer_index].needs_update = true;
    }
}

} // namespace vipe