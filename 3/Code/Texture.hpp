//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <Eigen/Eigen>
#include <opencv2/opencv.hpp>
class Texture {
private:
	cv::Mat image_data;

	inline cv::Vec3b lerp(float x, cv::Vec3b& v0, cv::Vec3b& v1) {
		return v0 + x * (v1 - v0);
	}

public:
	Texture(const std::string& name)
	{
		image_data = cv::imread(name);
		//if (image_data.empty()) {
		//    throw std::runtime_error("Failed to load image: " + name);
		//}
		cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
		width = image_data.cols;
		height = image_data.rows;
	}

	int width, height;

	Eigen::Vector3f getColor(float u, float v)
	{
		//if (v < 0 || v>1)std::cout << "u: " << u << ", v: " << v << std::endl;

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

		int x = floor(u_img);
		int y = floor(v_img);
		int x1 = std::clamp(x + 1, 0, width);
		int y1 = std::clamp(y + 1, 0, height);
		float t = u_img - x;
		float s = v_img - y;
		cv::Vec3b color1 = image_data.at<cv::Vec3b>(y, x);
		cv::Vec3b color2 = image_data.at<cv::Vec3b>(y, x1);
		cv::Vec3b color3 = image_data.at<cv::Vec3b>(y1, x);
		cv::Vec3b color4 = image_data.at<cv::Vec3b>(y1, x1);
		cv::Vec3b color_lerp1 = lerp(t, color1, color2);
		cv::Vec3b color_lerp2 = lerp(t, color3, color4);
		cv::Vec3b color = lerp(s, color_lerp1, color_lerp2);
		return Eigen::Vector3f(color[0], color[1], color[2]);
	}
};
#endif //RASTERIZER_TEXTURE_H
