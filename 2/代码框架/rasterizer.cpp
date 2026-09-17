// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}


static bool insideTriangle(int x, int y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    Vector2f P(x + 0.5f, y + 0.5f);
    Vector2f A(_v[0].x(), _v[0].y());
    Vector2f B(_v[1].x(), _v[1].y());
    Vector2f C(_v[2].x(), _v[2].y());

    auto getCrossDir = [](const Vector2f& m, const Vector2f& n)
    {
        return (m.x() * n.y() - m.y() * n.x()) > 0 ? 1 : -1;
    };
    auto eAB = getCrossDir(P - A, B - A);
    auto eBC = getCrossDir(P - B, C - B);
    auto eCA = getCrossDir(P - C, A - C);
    
    return eAB == eBC && eBC == eCA;
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }

    if (enable_ssaa)
    {
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                int pixIndex = get_index(x, y);
                Eigen::Vector3f pixColor = Eigen::Vector3f::Zero();
                for (int i = 0; i < 4; i++)
                {
                    int sampleIndex = pixIndex * 4 + i;
                    pixColor += frame_buf_ssaa[sampleIndex];
                }
                pixColor /= 4.0f;
                set_pixel(Vector3f(x, y, 0), pixColor);
            }
        }
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle

    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.

    auto a = t.v[0], b = t.v[1], c = t.v[2];
    int xMin = (int)std::floor(std::min({ a.x(), b.x(), c.x() })); xMin = std::max(xMin, 0);
    int xMax = (int)std::ceil(std::max({ a.x(), b.x(), c.x() })); xMax = std::min(xMax, width - 1);
    int yMin = (int)std::floor(std::min({ a.y(), b.y(), c.y() })); yMin = std::max(yMin, 0);
    int yMax = (int)std::ceil(std::max({ a.y(), b.y(), c.y() })); yMax = std::min(yMax, height - 1);

    const std::array<std::pair<float, float>, 4> samples = { {
        {0.25f, 0.25f},
        {0.25f, 0.75f},
        {0.75f, 0.25f},
        {0.75f, 0.75f}
    } };

    for (int x = xMin; x <= xMax; x++)
    {
        for (int y = yMin; y <= yMax; y++)
        {
            if (enable_ssaa)
            {
                int pixIndex = get_index(x, y);
                for (int i = 0; i < 4; i++)
                {
                    auto [alpha, beta, gamma] = computeBarycentric2D(x + samples[i].first, y + samples[i].second, t.v);
                    if (alpha >= 0 && beta >= 0 && gamma >= 0)
                    {
                        float z = alpha * a.z() + beta * b.z() + gamma * c.z();
                        int sampleIndex = pixIndex * 4 + i;
                        if (z < depth_buf_ssaa[sampleIndex])
                        {
                            depth_buf_ssaa[sampleIndex] = z;
                            frame_buf_ssaa[sampleIndex] = t.getColor();
                        }
                    }
                }
            }
            else 
            {
                auto [alpha, beta, gamma] = computeBarycentric2D(x + 0.5, y + 0.5, t.v);
                if (alpha >= 0 && beta >= 0 && gamma >= 0)
                {
                    float z = alpha * a.z() + beta * b.z() + gamma * c.z();
                    int index = get_index(x, y);
                    if (z < depth_buf[index])
                    {
                        depth_buf[index] = z;
                        set_pixel(Vector3f(x, y, z), t.getColor());
                    }
                }
            }
        }
    }
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{ 0, 0, 0 });
        std::fill(frame_buf_ssaa.begin(), frame_buf_ssaa.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
        std::fill(depth_buf_ssaa.begin(), depth_buf_ssaa.end(), std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    frame_buf_ssaa.resize(w * h * 4);
    depth_buf.resize(w * h);
    depth_buf_ssaa.resize(w * h * 4);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;

}

// clang-format on