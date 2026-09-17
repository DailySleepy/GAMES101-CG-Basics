//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <Eigen/Eigen>
#include <opencv2/opencv.hpp>
class Texture{
private:
    cv::Mat image_data;

    inline cv::Vec3f lerp(const cv::Vec3f& a, const cv::Vec3f& b, float t)
    {
        return a + t * (b - a);
    }

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        if (image_data.empty()) {
            std::cerr << "Error: Texture failed to load from path: " << name << std::endl;
            width = 0;
            height = 0;
            return;
        }
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }


    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        u = std::clamp(u, 0.0f, 1.0f);
        v = std::clamp(v, 0.0f, 1.0f);
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

    Eigen::Vector3f getColorBilinear(float u, float v)
    {
        u = std::clamp(u, 0.0f, 1.0f);
        v = std::clamp(v, 0.0f, 1.0f);
        auto u_img = u * width;
        auto v_img = (1 - v) * height;

        float x = std::max(u_img - 0.5f, 0.0f);
        float y = std::max(v_img - 0.5f, 0.0f);

        int x0 = static_cast<int>(std::floor(x));
        int y0 = static_cast<int>(std::floor(y));
        int x1 = std::min(x0 + 1, width - 1);
        int y1 = std::min(y0 + 1, height - 1);

        float s = x - std::floor(x);
        float t = y - std::floor(y);

        cv::Vec3f color00 = image_data.at<cv::Vec3b>(y0, x0);
        cv::Vec3f color01 = image_data.at<cv::Vec3b>(y0, x1);
        cv::Vec3f color10 = image_data.at<cv::Vec3b>(y1, x0);
        cv::Vec3f color11 = image_data.at<cv::Vec3b>(y1, x1);

        cv::Vec3f colorRow0 = lerp(color00, color01, s);
        cv::Vec3f colorRow1 = lerp(color10, color11, s);
        cv::Vec3f color = lerp(colorRow0, colorRow1, t);

        return Eigen::Vector3f(color[0], color[1], color[2]);
    }
};
#endif //RASTERIZER_TEXTURE_H
