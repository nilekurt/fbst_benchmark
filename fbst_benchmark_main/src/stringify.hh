#ifndef _STRINGIFY_HH_
#define _STRINGIFY_HH_

#include <array>
#include <charconv>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

template<typename T>
inline auto
stringify(const std::vector<T> & values)
    -> std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>>,
                        std::optional<std::string>>
{
    using U = std::decay_t<T>;

    std::stringstream ss{};
    ss << '{';
    const auto end = values.cend();
    for (auto it = values.cbegin(); it != end;) {
        if constexpr (std::is_integral_v<U>) {
            static std::array<char, std::numeric_limits<U>::digits + 2> chars{};
            auto [end_ptr, ec] = std::to_chars(chars.begin(), chars.end(), *it);
            if (ec != std::errc{} || end_ptr >= chars.end()) {
                return std::nullopt;
            }
            *end_ptr = '\0';
            ss << chars.data();
        } else {
            ss << std::to_string(*it);
        }

        if (++it != end) {
            ss << ", ";
        }
    }
    ss << '}';

    return std::make_optional(ss.str());
}

#endif // _STRINGIFY_HH_
