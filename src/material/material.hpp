#pragma once

#include "../math/vec3.hpp"
#include "texture.hpp"

#include <memory>

struct Material {
    std::shared_ptr<Texture> albedo_texture{std::make_shared<SolidColor>(Color(1.0f, 1.0f, 1.0f))};
    std::shared_ptr<Texture> emission_texture{std::make_shared<SolidColor>(Color(0.0f, 0.0f, 0.0f))};
    float metallic{0.0f};
    float roughness{0.5f};
    float transmission{0.0f};
    float ior{1.5f};
    Color absorption_coefficient{0.0f, 0.0f, 0.0f};

    Color sample_albedo(float u, float v, const Point3& p) const {
        if (!albedo_texture) {
            return Color(1.0f, 1.0f, 1.0f);
        }
        return albedo_texture->value(u, v, p);
    }

    Color sample_emission(float u, float v, const Point3& p) const {
        if (!emission_texture) {
            return Color(0.0f, 0.0f, 0.0f);
        }
        return emission_texture->value(u, v, p);
    }
};
