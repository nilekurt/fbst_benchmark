#include "fbst.hh"
#include "stringify.hh"

#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include <variant>

namespace {

template<typename T>
auto
single_rand() -> T
{
    static const auto now = std::chrono::system_clock::now();
    static const auto since_epoch =
        std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch())
            .count();
    static std::mt19937 generator{
        static_cast<std::mt19937::result_type>(since_epoch)};

    static std::uniform_int_distribution<T> dist(0,
                                                 std::numeric_limits<T>::max());

    return dist(generator);
}

template<typename Size, typename Generator>
auto
make_numbers(Size n, Generator && g)
    -> std::vector<std::invoke_result_t<Generator>>
{
    std::vector<std::invoke_result_t<Generator>> result{};
    result.reserve(n);

    std::generate_n(std::back_inserter(result), n, std::forward<Generator>(g));

    return result;
}

} // namespace

template<typename T>
class Tree {
    std::unique_ptr<Tree> left_;
    std::unique_ptr<Tree> right_;
    T                     value_;

public:
    template<typename U>
    Tree(std::unique_ptr<Tree> && left,
         std::unique_ptr<Tree> && right,
         U &&                     value)
        : left_{std::move(left)},
          right_{std::move(right)},
          value_{std::forward<U>(value)}
    {
    }

    struct IndexRef {
        size_t                          index;
        std::reference_wrapper<const T> ref;
    };

    [[nodiscard]] auto
    asVector(std::size_t index = 0) -> std::vector<IndexRef>
    {
        std::vector<IndexRef> result{};

        result.emplace_back(IndexRef{index, std::cref(value_)});

        if (left_) {
            auto l = left_->asVector(index * 2 + 1);
            result.insert(result.end(), l.begin(), l.end());
        }

        if (right_) {
            auto r = right_->asVector(index * 2 + 2);
            result.insert(result.end(), r.begin(), r.end());
        }

        return result;
    }
};

template<typename Iterator>
using IterVal = typename std::iterator_traits<Iterator>::value_type;

template<typename Iterator>
auto
make_balanced_tree(Iterator && begin, Iterator && end)
    -> std::unique_ptr<Tree<IterVal<Iterator>>>
{
    using T = IterVal<Iterator>;

    if (begin == end) {
        return nullptr;
    }

    auto midpoint = begin + std::distance(begin, end) / 2;
    if (midpoint == begin) {
        return std::make_unique<Tree<T>>(
            nullptr, nullptr, std::forward<decltype(*begin)>(*begin));
    }
    auto midpoint_2 = midpoint + 1;

    auto && val = *midpoint;

    return std::make_unique<Tree<T>>(
        make_balanced_tree(std::forward<Iterator>(begin),
                           std::forward<decltype(midpoint)>(midpoint)),
        make_balanced_tree(std::forward<decltype(midpoint_2)>(midpoint_2),
                           std::forward<Iterator>(end)),
        std::forward<decltype(val)>(val));
}

auto
main(int argc, char ** argv) -> int
{
    try {
        if (argc != 4) {
            std::cerr
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                << "Usage: " << argv[0]
                << " <number_of_entries> <entry_size> <number_of_queries>\n";
            return -1;
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto num_values = std::strtoumax(argv[1], nullptr, 10);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto entry_size_index = std::strtoumax(argv[2], nullptr, 10);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto num_queries = std::strtoumax(argv[3], nullptr, 10);

        using IntegerVar = std::variant<uint8_t, uint16_t, uint32_t, uint64_t>;

        auto entry_type = std::invoke([entry_size_index] {
            switch (entry_size_index) {
                case 1: return IntegerVar{std::in_place_type_t<uint8_t>{}};
                case 2: return IntegerVar{std::in_place_type_t<uint16_t>{}};
                case 4: return IntegerVar{std::in_place_type_t<uint32_t>{}};
                case 8: // Fallthrough
                default: return IntegerVar{std::in_place_type_t<uint64_t>{}};
            }
        });

        return std::visit(
            [=](const auto & dummy) {
                using DataType = std::decay_t<decltype(dummy)>;

                if (num_values == 0) {
                    std::cerr << "Table must have at least 1 entry\n";
                    return -1;
                }

                auto values = make_numbers(num_values, single_rand<DataType>);
                std::sort(values.begin(), values.end());

                auto tree = make_balanced_tree(values.cbegin(), values.cend());

                auto arrayified = tree->asVector();

                std::sort(
                    arrayified.begin(),
                    arrayified.end(),
                    [](auto & a, auto & b) { return a.index <= b.index; });

                std::vector<DataType> result{};
                result.reserve(num_values);

                std::transform(arrayified.begin(),
                               arrayified.end(),
                               std::back_inserter(result),
                               [](auto & x) { return x.ref; });

#ifndef NDEBUG
                // The last value of a sorted array is the maximum element
                const auto max = static_cast<double>(values.back());

                std::vector<double> weights{};
                weights.reserve(num_values);

                std::transform(result.begin(),
                               result.end(),
                               std::back_inserter(weights),
                               [max](const auto x) {
                                   return static_cast<double>(x) / max;
                               });

                auto maybe_value_str = stringify(values);
                auto maybe_result_str = stringify(result);
                auto maybe_weights_str = stringify(weights);

                if (!maybe_value_str.has_value() ||
                    !maybe_result_str.has_value() ||
                    !maybe_weights_str.has_value()) {
                    std::cerr << "Unable to compose output strings\n";
                    return -1;
                }

                std::cout << *maybe_value_str << '\n'
                          << *maybe_result_str << '\n'
                          << *maybe_weights_str << '\n';
#endif

                auto queries = make_numbers(num_queries, single_rand<DataType>);

                // Noop
                ///////////////////////
                auto start = std::chrono::high_resolution_clock::now();
                for (auto q : queries) {
                    __asm__ volatile("" : "+g"(q) : :);
                }
                auto       end = std::chrono::high_resolution_clock::now();
                const auto delta_noop = (end - start).count();

                // Linear search
                ///////////////////////
                start = std::chrono::high_resolution_clock::now();
                for (const auto q : queries) {
                    auto found =
                        std::find_if(values.begin(),
                                     values.end(),
                                     [q](const auto val) { return q <= val; });
                    __asm__ volatile("" : "+g"(found) : :);
                }
                end = std::chrono::high_resolution_clock::now();
                const auto delta_linear = (end - start).count();

                // Binary search
                ///////////////////////
                start = std::chrono::high_resolution_clock::now();
                for (const auto q : queries) {
                    auto found =
                        std::lower_bound(values.begin(), values.end(), q);
                    __asm__ volatile("" : "+g"(found) : :);
                }
                end = std::chrono::high_resolution_clock::now();
                const auto delta_binary = (end - start).count();

                // Flattened BST search (iterators)
                ///////////////////////
                start = std::chrono::high_resolution_clock::now();
                for (const auto q : queries) {
                    auto found = fbst::search(result.begin(), result.end(), q);
                    __asm__ volatile("" : "+g"(found) : :);
                }
                end = std::chrono::high_resolution_clock::now();
                const auto delta_fbst_it = (end - start).count();

                // Flattened BST search (references)
                ///////////////////////
                start = std::chrono::high_resolution_clock::now();
                for (const auto q : queries) {
                    auto found = fbst::search(result, q);
                    __asm__ volatile("" : "+g"(found) : :);
                }
                end = std::chrono::high_resolution_clock::now();
                const auto delta_fbst_ref = (end - start).count();

                auto avg = [num_queries](const auto x) {
                    return static_cast<double>(x) /
                           static_cast<double>(num_queries);
                };

                auto norm = [delta_noop](const auto x) {
                    return static_cast<double>(x) /
                           static_cast<double>(delta_noop);
                };

                std::map<decltype(delta_noop), std::string> tagged_time{
                    {delta_noop, "Noop"},
                    {delta_linear, "Linear"},
                    {delta_binary, "Binary"},
                    {delta_fbst_it, "FBST (iter)"},
                    {delta_fbst_ref, "FBST (ref)"}};

                const auto longest_tag_length = std::transform_reduce(
                    tagged_time.begin(),
                    tagged_time.end(),
                    std::string::size_type{0},
                    [](auto a, auto b) { return std::max(a, b); },
                    [](const auto & val) { return val.second.size(); });

                constexpr auto precision = 2;
                const auto     tag_width = longest_tag_length;
                constexpr auto avg_width = precision + 6;
                constexpr auto unit_width = 5;
                constexpr auto norm_width = precision + 6;

                for (auto & [delta, tag] : tagged_time) {
                    std::cout << std::setprecision(precision)
                              << std::setw(tag_width) << tag << ':'
                              << std::fixed << std::right
                              << std::setw(avg_width) << avg(delta)
                              << std::setw(unit_width) << " ns," << std::right
                              << std::setw(norm_width) << norm(delta) << "x\n";
                }

                return 0;
            },
            entry_type);
    } catch (const std::exception & e) {
        std::cerr << e.what() << '\n';
        return -1;
    }
}