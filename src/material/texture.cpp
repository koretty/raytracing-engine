#include "texture.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

namespace {

inline bool read_next_token(std::istream& input, std::string& token) {
    while (input >> token) {
        if (!token.empty() && token[0] == '#') {
            input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        return true;
    }
    return false;
}

inline bool parse_int(const std::string& token, int& value) {
    try {
        size_t parsed = 0;
        const long long raw = std::stoll(token, &parsed);
        if (parsed != token.size()) {
            return false;
        }
        if (raw < static_cast<long long>(std::numeric_limits<int>::min()) ||
            raw > static_cast<long long>(std::numeric_limits<int>::max())) {
            return false;
        }
        value = static_cast<int>(raw);
        return true;
    } catch (...) {
        return false;
    }
}

inline uint8_t scale_to_u8(int sample, int max_value) {
    if (max_value <= 0) {
        return 0;
    }
    const int clamped = std::clamp(sample, 0, max_value);
    const float normalized = static_cast<float>(clamped) / static_cast<float>(max_value);
    return static_cast<uint8_t>(std::round(normalized * 255.0f));
}

} // namespace

Color CheckerTexture::value(float u, float v, const Point3& p) const {
    const int ix = static_cast<int>(std::floor(p.x * inv_scale));
    const int iy = static_cast<int>(std::floor(p.y * inv_scale));
    const int iz = static_cast<int>(std::floor(p.z * inv_scale));

    // Integer parity avoids branch-heavy sign checks.
    const int pattern = (ix ^ iy ^ iz) & 1;
    const Texture* even_ptr = even.get();
    const Texture* odd_ptr = odd.get();
    const Texture* fallback = even_ptr ? even_ptr : odd_ptr;
    if (!fallback) {
        return Color(0.0f, 0.0f, 0.0f);
    }

    const Texture* candidates[2] = {
        even_ptr ? even_ptr : fallback,
        odd_ptr ? odd_ptr : fallback,
    };
    return candidates[pattern]->value(u, v, p);
}

bool ImageTexture::load(const std::string& file_path) {
    width = 0;
    height = 0;
    channels = 0;
    row_stride = 0;
    pixels.clear();

    std::ifstream input(file_path, std::ios::binary);
    if (!input) {
        return false;
    }

    std::string magic;
    if (!read_next_token(input, magic)) {
        return false;
    }

    if (magic != "P3" && magic != "P6") {
        return false;
    }

    std::string token;
    int parsed_width = 0;
    int parsed_height = 0;
    int max_value = 0;

    if (!read_next_token(input, token) || !parse_int(token, parsed_width) || parsed_width <= 0) {
        return false;
    }
    if (!read_next_token(input, token) || !parse_int(token, parsed_height) || parsed_height <= 0) {
        return false;
    }
    if (!read_next_token(input, token) || !parse_int(token, max_value) || max_value <= 0) {
        return false;
    }

    const size_t pixel_count = static_cast<size_t>(parsed_width) * static_cast<size_t>(parsed_height);
    const size_t byte_count = pixel_count * 3u;
    std::vector<uint8_t> loaded_pixels(byte_count, 0u);

    if (magic == "P3") {
        for (size_t i = 0; i < byte_count; ++i) {
            if (!read_next_token(input, token)) {
                return false;
            }

            int component = 0;
            if (!parse_int(token, component)) {
                return false;
            }

            loaded_pixels[i] = scale_to_u8(component, max_value);
        }
    } else {
        if (max_value > 255) {
            return false;
        }

        char separator = 0;
        input.get(separator);
        if (!input || !std::isspace(static_cast<unsigned char>(separator))) {
            return false;
        }
        if (separator == '\r' && input.peek() == '\n') {
            input.get(separator);
        }

        input.read(reinterpret_cast<char*>(loaded_pixels.data()), static_cast<std::streamsize>(byte_count));
        if (!input) {
            return false;
        }

        if (max_value != 255) {
            for (uint8_t& component : loaded_pixels) {
                component = scale_to_u8(static_cast<int>(component), max_value);
            }
        }
    }

    width = parsed_width;
    height = parsed_height;
    channels = 3;
    row_stride = static_cast<size_t>(width) * static_cast<size_t>(channels);
    pixels = std::move(loaded_pixels);
    return true;
}

Color ImageTexture::value(float u, float v, const Point3& p) const {
    (void)p;

    if (!is_valid()) {
        return Color(1.0f, 0.0f, 1.0f);
    }

    const float uu = std::clamp(u, 0.0f, 1.0f);
    const float vv = 1.0f - std::clamp(v, 0.0f, 1.0f);

    const int x = std::min(static_cast<int>(uu * static_cast<float>(width)), width - 1);
    const int y = std::min(static_cast<int>(vv * static_cast<float>(height)), height - 1);

    return get_pixel(x, y);
}
