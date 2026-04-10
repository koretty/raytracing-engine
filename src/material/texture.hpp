#pragma once

#include "../math/vec3.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Texture {
public:
    virtual ~Texture() = default;
    virtual Color value(float u, float v, const Point3& p) const = 0;
};

class SolidColor final : public Texture {
public:
    SolidColor() = default;
    explicit SolidColor(const Color& color) : color_value(color) {}
    SolidColor(float r, float g, float b) : color_value(r, g, b) {}

    Color value(float u, float v, const Point3& p) const override {
        (void)u;
        (void)v;
        (void)p;
        return color_value;
    }

private:
    Color color_value{0.0f, 0.0f, 0.0f};
};

class CheckerTexture final : public Texture {
public:
    CheckerTexture(std::shared_ptr<Texture> even_texture,
                   std::shared_ptr<Texture> odd_texture,
                   float scale = 1.0f)
        : even(std::move(even_texture)),
          odd(std::move(odd_texture)),
          inv_scale(scale > 0.0f ? (1.0f / scale) : 1.0f) {
    }

    CheckerTexture(const Color& even_color, const Color& odd_color, float scale = 1.0f)
        : CheckerTexture(std::make_shared<SolidColor>(even_color),
                         std::make_shared<SolidColor>(odd_color),
                         scale) {
    }

    Color value(float u, float v, const Point3& p) const override;

private:
    std::shared_ptr<Texture> even;
    std::shared_ptr<Texture> odd;
    float inv_scale{1.0f};
};

class ImageTexture final : public Texture {
public:
    ImageTexture() = default;
    explicit ImageTexture(const std::string& file_path) {
        load(file_path);
    }

    bool load(const std::string& file_path);
    bool is_valid() const {
        return width > 0 && height > 0 && channels >= 3 && !pixels.empty();
    }

    Color value(float u, float v, const Point3& p) const override;

    int get_width() const { return width; }
    int get_height() const { return height; }

private:
    int width{0};
    int height{0};
    int channels{0};
    size_t row_stride{0};
    std::vector<uint8_t> pixels;

    inline size_t pixel_index(int x, int y) const {
        return static_cast<size_t>(y) * row_stride + static_cast<size_t>(x) * static_cast<size_t>(channels);
    }

    // Access abstraction point for future Morton/tiled layout migration.
    inline Color get_pixel(int x, int y) const {
        const size_t idx = pixel_index(x, y);
        constexpr float scale = 1.0f / 255.0f;
        return Color(static_cast<float>(pixels[idx + 0u]) * scale,
                     static_cast<float>(pixels[idx + 1u]) * scale,
                     static_cast<float>(pixels[idx + 2u]) * scale);
    }
};
