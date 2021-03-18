#pragma once

#include <map>
#include <memory>
#include <optional>
#include <limits>

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

      public:
        using key_type = storage_type::key_type;
        using mapped_type = storage_type::mapped_type;

        range_map(const key_type& value)
        {
            _storage.insert(std::end(_storage), std::pair{std::numeric_limits<key_type>::lowest(), value});
        }

        using key_range_t = std::pair<key_type, key_type>;
        void assign(key_range_t&& key_range, mapped_type&& value)
        {
            const auto& [keyBegin, keyEnd] = key_range;
            if (keyBegin >= keyEnd)
            {
                return;
            }

            // select target range
            auto range_begin = _storage.lower_bound(keyBegin);
            auto range_end = _storage.upper_bound(keyEnd);

            // save after-range value
            auto value_after_range = std::move(std::prev(range_end)->second);

            // erase existing nodes in range
            range_end = _storage.erase(range_begin, range_end);

            // insert begin
            if (_storage.size() == 0 or std::prev(range_end)->second not_eq value)
                range_end = _storage.insert_or_assign(range_end, keyBegin, value);
            // insert end
            if (value_after_range not_eq value)
                range_end = _storage.insert_or_assign(range_end, keyEnd, std::move(value_after_range));
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

namespace gcl::container::test::interval_map
{

}