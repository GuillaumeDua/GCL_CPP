#pragma once

#include <map>
#include <memory>
#include <optional>
#include <limits>
#include <gcl/algorithms/ranges.hpp>

namespace gcl::container
{

    template <
        class Key,
        class T,
        class Compare = std::less<Key>,
        class Allocator = std::allocator<std::pair<const Key, T>>>
    struct range_map {
        using storage_type = typename std::map<Key, T, Compare, Allocator>;

      private:
        storage_type _storage;

        void ensure_is_valid()
        {
            if (std::size(_storage) == 0 or std::begin(_storage).first != std::numeric_limits<key_type>::lowest())
                throw std::runtime_error{"gcl::container::range_map::ensure_is_valid"};
            for (auto it = std::next(std::cbegin(_storage)); it != std::cend(_storage); ++it)
            {
                if (std::prev(it)->first == it->first)
                    throw std::runtime_error{"gcl::container::range_map::ensure_is_valid"};
            }
        }

      public:
        using key_type = typename storage_type::key_type;
        using mapped_type = typename storage_type::mapped_type;
        using value_type = typename storage_type::value_type;

        range_map(mapped_type&& value)
        {
            _storage.insert(
                std::end(_storage),
                std::pair{std::numeric_limits<key_type>::lowest(), std::forward<mapped_type>(value)});
        }
        range_map(const mapped_type& value)
            : range_map(mapped_type{value})
        {}

        using value_argument_type = std::pair<std::decay_t<key_type>, mapped_type>;
        range_map(mapped_type&& value, std::initializer_list<value_argument_type> arguments)
            : range_map{std::forward<mapped_type>(value)}
        {
            auto args = std::vector<value_argument_type>{std::begin(arguments), std::end(arguments)};
            std::sort(std::begin(args), std::end(args), [comparator = Compare{}](const auto& lhs, const auto& rhs) {
                return comparator(lhs.first, rhs.first);
            });

            // deduplicate copy of sorted elements
            std::remove_copy_if(
                std::begin(args),
                std::end(args),
                std::inserter(_storage, std::end(_storage)),
                [previous_value = std::optional<mapped_type>{std::nullopt}, this](const auto& element) mutable {

                const bool result = previous_value
                        ? *previous_value == element.second
                        : std::cbegin(_storage)->second == element.second;
                    previous_value = element.second;
                    return result;
            });
        }

        auto operator==(const storage_type& arg) { return _storage == arg; }
        auto operator==(const range_map& arg) { return _storage == arg.storage(); }

        using key_range_t = std::pair<key_type, key_type>;
        void assign(key_range_t&& key_range, mapped_type&& value)
        {   // there should be some extract()  here in some cases
            const auto& [keyBegin, keyEnd] = key_range;
            if (keyBegin >= keyEnd)
            {
                return;
            }

            // select target range
            auto range_begin = _storage.lower_bound(keyBegin);
            auto range_end = _storage.upper_bound(keyEnd);

            // save after-range value
            auto mapped_after_range = [&]() -> std::optional<mapped_type> {
                auto candidate = std::prev(range_end);

                if (value == candidate->second)
                    return std::nullopt;
                if constexpr (std::movable<mapped_type>)
                {
                    return gcl::algorithms::ranges::is_in_range(range_begin, range_end, candidate) // will be erase ?
                               ? std::move(candidate->second)
                               : candidate->second;
                }
                else
                    return std::prev(range_end)->second;
            }();

            // erase existing nodes in range
            range_end = _storage.erase(range_begin, range_end);

            // insert begin
            if (_storage.size() == 0 or std::prev(range_end)->second not_eq value)
                range_end = _storage.insert_or_assign(range_end, keyBegin, std::forward<mapped_type>(value));
            // insert end
            if (mapped_after_range)
                range_end = _storage.insert_or_assign(range_end, keyEnd, std::move(*mapped_after_range));
        }

        void assign_last(mapped_type&& value)
        {
            // todo : how to assign last value ?
        }

        const auto& at(const key_type& key) const { return (--_storage.upper_bound(key))->second; }
        const auto& storage() const { return _storage; }
    };

    template <
        class Key,
        class T,
        class Compare = std::less<Key>,
        class Allocator = std::allocator<std::pair<const Key, T>>>
    using interval_map = range_map<Key, T, Compare, Allocator>;
}

#include <stdexcept>
namespace gcl::container::test::interval_map
{
    void test_values()
    {
        auto value = gcl::container::range_map<unsigned int, char>('a');
        {
            const auto expected =
                decltype(value)::storage_type{{std::numeric_limits<decltype(value)::key_type>::lowest(), 'a'}};

            if (value.storage() not_eq expected)
                throw std::runtime_error{"test::interval_map : default constructed"};

            value.assign({3U, 5U}, 'a');
            if (value.storage() not_eq expected)
                throw std::runtime_error{"test::interval_map : no-op assign"};

            value.assign({std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::min()}, '.');
            if (value.storage() not_eq expected)
                throw std::runtime_error{"test::interval_map : no-op assign [max, min) "};
        }
        value.assign({5U, 10U}, 'X');
        {
            const auto expected = decltype(value)::storage_type{
                {std::numeric_limits<decltype(value)::key_type>::lowest(), 'a'}, {5U, 'X'}, {10U, 'a'}};
            if (value.storage() not_eq expected)
                throw std::runtime_error{"test::interval_map : split range"};
        }
        value.assign({4U, 9U}, 'X');
        {
            const auto expected = decltype(value)::storage_type{
                {std::numeric_limits<decltype(value)::key_type>::lowest(), 'a'}, {4U, 'X'}, {10U, 'a'}};
            if (value.storage() not_eq expected)
                throw std::runtime_error{"test::interval_map : extend middle range, bottom"};
        }
        value.assign({5U, 11U}, 'X');
        {
            const auto expected = decltype(value)::storage_type{
                {std::numeric_limits<decltype(value)::key_type>::lowest(), 'a'}, {4U, 'X'}, {11U, 'a'}};
            if (value.storage() not_eq expected)
                throw std::runtime_error{"test::interval_map : extend middle range, top"};
        }

        value.assign({std::numeric_limits<unsigned int>::min(), std::numeric_limits<unsigned int>::max()}, '_');
        {
            const auto expected =
                decltype(value)::storage_type{
                    {std::numeric_limits<decltype(value)::key_type>::lowest(), '_'},
                    {std::numeric_limits<unsigned int>::max(), 'a'}
            };
            if (value.storage() not_eq expected)
                throw std::runtime_error{"test::interval_map : [min, max) override"};
        }
    }
    void test_rvalues()
    { 
        auto value = gcl::container::range_map<unsigned int, std::string>("a");
        value.assign({5U, 10U}, "X");
        {
            const auto expected = decltype(value)::storage_type{
                {std::numeric_limits<decltype(value)::key_type>::lowest(), "a"}, {5U, "X"}, {10U, "a"}};
            if (value.storage() not_eq expected)
            {
                for (const auto & [el_key, el_value] : value.storage())
                {
                    std::cout << " - [" << el_key << "] -> [" << el_value << "]\n";
                }
                throw std::runtime_error{"test::interval_map (rvalue): split range"};
            }
        }
        value.assign({4U, 9U}, "X");
        {
            const auto expected = decltype(value)::storage_type{
                {std::numeric_limits<decltype(value)::key_type>::lowest(), "a"}, {4U, "X"}, {10U, "a"}};
            if (value.storage() not_eq expected)
                throw std::runtime_error{"test::interval_map (rvalue): extend middle range, bottom"};
        }
        value.assign({5U, 11U}, "X");
        {
            const auto expected = decltype(value)::storage_type{
                {std::numeric_limits<decltype(value)::key_type>::lowest(), "a"}, {4U, "X"}, {11U, "a"}};
            if (value.storage() not_eq expected)
                throw std::runtime_error{"test::interval_map (rvalue): extend middle range, top"};
        }

        value.assign({std::numeric_limits<unsigned int>::min(), std::numeric_limits<unsigned int>::max()}, "_");
        {
            const auto expected = decltype(value)::storage_type{
                {std::numeric_limits<decltype(value)::key_type>::lowest(), "_"},
                {std::numeric_limits<unsigned int>::max(), "a"}};
            if (value.storage() not_eq expected)
                throw std::runtime_error{"test::interval_map (rvalue): [min, max) override"};
        }
    }
    void test_constructors()
    {
        {
            auto       value = gcl::container::range_map<unsigned int, char>('a');
            const auto expected =
                decltype(value)::storage_type{{std::numeric_limits<decltype(value)::key_type>::lowest(), 'a'}};
            if (value.storage() not_eq expected)
                throw std::runtime_error{"test::interval_map : default constructed (rvalue)"};
        }
        {
            const auto argument = 'a';
            auto       value = gcl::container::range_map<unsigned int, char>(argument);
            const auto expected =
                decltype(value)::storage_type{{std::numeric_limits<decltype(value)::key_type>::lowest(), 'a'}};
            if (value.storage() not_eq expected)
                throw std::runtime_error{"test::interval_map : default constructed (const-ref)"};
        }
        {
            auto       value = gcl::container::range_map<unsigned int, char>{'a', {{13, 'b'}, {42, 'c'}}};
            const auto expected = decltype(value)::storage_type{
                {std::numeric_limits<decltype(value)::key_type>::lowest(), 'a'}, {13, 'b'}, {42, 'c'}};
            if (value.storage() not_eq expected)
                throw std::runtime_error{"test::interval_map : initializer_list construct (no duplicates)"};
        }
        {
            auto       value = gcl::container::range_map<unsigned int, char>{'a', {{42, 'c'}, {13, 'b'}}};
            const auto expected = decltype(value)::storage_type{
                {std::numeric_limits<decltype(value)::key_type>::lowest(), 'a'}, {13, 'b'}, {42, 'c'}};
            if (value.storage() not_eq expected)
                throw std::runtime_error{"test::interval_map : initializer_list construct (no duplicates, sort)"};
        }
        {
            auto value = gcl::container::range_map<unsigned int, char>{
                'a', {{42, 'c'}, {2, 'a'}, {13, 'b'}, {40, 'b'}, {43, 'c'}}};
            const auto expected = decltype(value)::storage_type{
                {std::numeric_limits<decltype(value)::key_type>::lowest(), 'a'}, {13, 'b'}, {42, 'c'}};
            if (value.storage() not_eq expected)
                throw std::runtime_error{"test::interval_map : initializer_list construct (duplicates, sort)"};
        }
    }

    void test()
    {
        test_constructors();
        test_values();
        test_rvalues();
    }
}