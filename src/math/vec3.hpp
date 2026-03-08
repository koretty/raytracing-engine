#pragma once

#include <cmath>

class vec3{
public:

    union{
        struct{double x,y,z;};
        double e[3];
    };

    vec3() : x(0), y(0), z(0) {}
    vec3(double x, double y, double z) : x(x), y(y), z(z) {}

    vec3 operator-() const { return vec3(-x, -y, -z); }
    double operator[](int i) const { return e[i]; }
    double& operator[](int i) { return e[i]; }

    vec3& operator+=(const vec3& v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    vec3& operator*=(double t) {
        x *= t;
        y *= t;
        z *= t;
        return *this;
    }

    vec3& operator/=(double t) {
        return *this *= 1 / t;
    }

    double length() const {
        return std::sqrt(length_squared());
    }

    double length_squared() const {
        return x*x + y*y + z*z;
    }
};

inline vec3 operator+(const vec3& u, const vec3& v) {
    return vec3(u.x + v.x, u.y + v.y, u.z + v.z);
}

inline vec3 operator-(const vec3& u, const vec3& v) {
    return vec3(u.x - v.x, u.y - v.y, u.z - v.z);
}

inline vec3 operator*(const vec3& u, const vec3& v) {
    return vec3(u.x * v.x, u.y * v.y, u.z * v.z);
}

inline vec3 operator*(double t, const vec3& v) {
    return vec3(t * v.x, t * v.y, t * v.z);
}

inline vec3 operator*(const vec3& v, double t) {
    return t * v;
}

inline vec3 operator/(vec3 v, double t) {
    return (1 / t) * v;
}

inline double dot(const vec3& u, const vec3& v) {
    return (u.x * v.x) + (u.y * v.y) + (u.z * v.z);
}

inline vec3 cross(const vec3& u, const vec3& v) {
    return vec3(u.y * v.z - u.z * v.y,
                u.z * v.x - u.x * v.z,
                u.x * v.y - u.y * v.x);
}

inline vec3 unit_vector(const vec3& v) {
    return v / v.length();
}

using point3 = vec3;   
using color = vec3;   