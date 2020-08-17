#include <array>
#include <iterator>
#include <type_traits>
#include <vector>

namespace fbst {

template<typename T, typename Size, typename U>
constexpr auto
search(T * data, Size size, U && val) noexcept
{
    Size pos{};
    Size last_pos{};
    do {
        last_pos = pos;
        if (data[pos] <= val) {
            pos = pos * 2 + 1;
        } else {
            pos = pos * 2 + 2;
        }
    } while (pos < size);
    return last_pos;
}

template<typename Iterator, typename U>
constexpr auto
search(Iterator && start, Iterator && end, U && val) -> std::enable_if_t<
    std::is_same_v<typename std::iterator_traits<Iterator>::value_type,
                   std::decay_t<U>>,
    Iterator>
{
    const auto size = std::distance(start, end);

    std::ptrdiff_t pos{};
    std::ptrdiff_t last_pos{};
    do {
        last_pos = pos;
        if (*(start + pos) <= val) {
            pos = pos * 2 + 1;
        } else {
            pos = pos * 2 + 2;
        }
    } while (pos < size);
    return start + last_pos;
}

template<typename T, typename U, std::size_t N>
constexpr auto
search(std::array<T, N> & data, U && val) noexcept
{
    return data[search(data.data(), data.size(), std::forward<U>(val))];
}

template<typename T, typename U, std::size_t N>
constexpr auto
search(const std::array<T, N> & data, U && val) noexcept
{
    return data[search(data.data(), data.size(), std::forward<U>(val))];
}

template<typename T, typename U>
constexpr auto
search(std::vector<T> & data, U && val) noexcept
{
    return data[search(data.data(), data.size(), std::forward<U>(val))];
}

template<typename T, typename U>
constexpr auto
search(const std::vector<T> & data, U && val) noexcept
{
    return data[search(data.data(), data.size(), std::forward<U>(val))];
}

} // namespace fbst