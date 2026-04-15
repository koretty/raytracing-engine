#pragma once

#include "../math/ray.hpp"
#include "../math/math_utils.hpp"
#include <cmath>
#include <numbers>

class Camera {
    Point3 origin;
    Point3 lookat;
    Vec3 up;
    Vec3 front;
    Vec3 right;
    Point3 lower_left_corner;
    float viewport_h;
    float viewport_v;
    float fov_deg;
    float aspect_ratio;
    float aperture;
    float lens_radius;
    float focus_dist;

    static Vec3 random_in_unit_disk() {
        while (true) {
            Vec3 p(random_float(-1.0f, 1.0f), random_float(-1.0f, 1.0f), 0.0f);
            if (p.length_squared() >= 1.0f) continue;
            return p;
        }
    }
public:
    Camera(
        const Point3& origin,
        const Point3& lookat,
        const Vec3& view_up,
        float fov_deg,
        float aspect_ratio,
        float aperture,
        float focus_dist
    ):
    origin(origin), 
    lookat(lookat),  
    fov_deg(fov_deg), 
    aspect_ratio(aspect_ratio), 
    aperture(aperture), 
    lens_radius(aperture * 0.5f),
    focus_dist(focus_dist){
        Vec3 forward = lookat - origin;
        if (forward.near_zero()) {
            forward = Vec3(0.0f, 0.0f, -1.0f);
        }
        front = unit_vector(forward);

        Vec3 up_hint = view_up.near_zero() ? Vec3(0.0f, 1.0f, 0.0f) : view_up;
        Vec3 right_candidate = cross(front, up_hint);
        if (right_candidate.near_zero()) {
            Vec3 fallback_up = (std::abs(front.y) < 0.999f)
                ? Vec3(0.0f, 1.0f, 0.0f)
                : Vec3(1.0f, 0.0f, 0.0f);
            right_candidate = cross(front, fallback_up);
        }
        right = unit_vector(right_candidate);

        Vec3 up_candidate = cross(right, front);
        up = up_candidate.near_zero() ? Vec3(0.0f, 1.0f, 0.0f) : unit_vector(up_candidate);
        float fov_rad = fov_deg * static_cast<float>(std::numbers::pi) / 180.0f;
        viewport_v = 2.0f * tan(fov_rad * 0.5f) * focus_dist;
        viewport_h = aspect_ratio * viewport_v;
        lower_left_corner = origin - viewport_h * 0.5f * right - viewport_v * 0.5f * up  + front * focus_dist;
    }

    Ray get_ray(float u, float v) const {
        Vec3 rd = lens_radius * random_in_unit_disk();
        Vec3 offset = right * rd.x + up * rd.y;

        Point3 ray_origin = origin + offset;
        Point3 pixel_on_focus_plane = lower_left_corner + u * viewport_h * right + v * viewport_v * up;
        return Ray(ray_origin, pixel_on_focus_plane - ray_origin);
    }
};