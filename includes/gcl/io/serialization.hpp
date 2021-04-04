#pragma once

// original POC : https://godbolt.org/z/P4a6voYqP

#include <gcl/io/policy.hpp>
#include <gcl/io/concepts.hpp>
#include <gcl/functional.hpp>
#include <gcl/cx/typeinfo.hpp>
#include <gcl/mp/pack_traits.hpp>
#include <gcl/mp/type_traits.hpp>

#include <unordered_map>
#include <functional>

// todo : add aggregate speciale rule

namespace gcl::io::serialization
{
    template <typename... Ts>
    struct signature {
        using storage_type = std::tuple<Ts...>;
        storage_type storage;

        constexpr signature(Ts&&... args)
            : storage{std::forward<Ts>(args)...}
        {}
        constexpr signature(const Ts&... args)
            : storage{std::forward<Ts>(args)...}
        {}
    };

    template <class io_policy = gcl::io::policy::binary>
    struct engine {

        template <typename on_deserialization_t>
        class in {
            using type_id_t = uint32_t;
            using storage_type = std::unordered_map<type_id_t, std::function<void()>>;
            std::istream&        input_stream;
            on_deserialization_t on_deserialize;
            storage_type         storage;

            template <gcl::io::concepts::serializable T>
            auto generate_type_handler()
            {
                return std::pair{
                    gcl::cx::typeinfo::hashcode<T>(), std::function<void()>{[this]() {
                        T value;
                        io_policy::read(input_stream, value);
                        if (not input_stream.good())
                            throw std::runtime_error{
                                "serialization::engine::deserialize (value) / storage::mapped::operator() "};
                        on_deserialize(std::move(value));
                    }}};
            }

          public:

            in(std::istream& input, on_deserialization_t&& cb)
                : input_stream{input}
                , on_deserialize{std::forward<decltype(cb)>(cb)}
            {
                if constexpr (gcl::functional::type_traits::is_overload_v<on_deserialization_t>)
                {
                    const auto register_signature = [this]<typename... Ts>(std::tuple<Ts...>)
                    {
                        static_assert(((not std::is_const_v<Ts>)&&...));
                        static_assert(((not std::is_reference_v<Ts>)&&...));

                        using handler_signature = signature<Ts...>;
                        storage.insert(
                            std::pair{gcl::cx::typeinfo::hashcode<handler_signature>(), std::function<void()>{[this]() {
                                          std::tuple<Ts...> arguments;
                                          const auto        read_one_argument = [this]<typename T>(T&& value) {

                                              io_policy::read(input_stream, std::forward<T>(value));
                                              if (not input_stream.good())
                                                  throw std::runtime_error{
                                                      "serialization::engine::deserialize (value) / "
                                                      "storage::mapped::operator() "};
                                          };
                                          [ this, &arguments, &
                                            read_one_argument ]<std::size_t... indexes>(std::index_sequence<indexes...>)
                                          {
                                              ((read_one_argument(std::move(std::get<indexes>(arguments)))), ...);
                                          }
                                          (std::make_index_sequence<std::tuple_size_v<decltype(arguments)>>{});

                                          std::apply(on_deserialize, std::move(arguments));
                                      }}});
                    };
                    [register_signature]<typename... Ts>(gcl::functional::overload<Ts...> &&)
                    {
                        using remove_cv_and_ref = gcl::mp::type_traits::merge_traits<std::remove_reference_t, std::decay_t>;

                        ((register_signature(gcl::mp::type_traits::transform_t<
                            gcl::mp::function_traits_t<decltype(&Ts::operator())>::template arguments,
                            remove_cv_and_ref::type>{})), ...);

                    }(std::forward<decltype(cb)>(cb));
                }
            }

            template <gcl::io::concepts::serializable ... Ts>
            in(std::istream& input, on_deserialization_t&& cb, std::tuple<Ts...>)
                : input_stream{input}
                , on_deserialize{std::forward<decltype(cb)>(cb)}
                , storage{generate_type_handler<Ts>()...}
            {}

            template <gcl::io::concepts::serializable ... Ts>
            void register_types()
            {
                const auto insert_if_not_exists = [this]<typename T>() {
                    if (not storage.contains(gcl::cx::typeinfo::hashcode_v<T>))
                        storage.insert(generate_type_handler<T>());
                };
                ((insert_if_not_exists.template operator()<Ts>()), ...);
            }
            template <typename... Ts>
            void unregister_types()
            {
                (storage.erase(gcl::cx::typeinfo::hashcode<Ts>()), ...);
            }

            void deserialize()
            {
                type_id_t type_id_value;
                io_policy::read(input_stream, type_id_value);

                if (input_stream.eof())
                    return;
                if (not input_stream.good())
                    throw std::runtime_error{"serialization::engine::deserialize (typeid)"};

                try
                {
                    auto& handler = storage.at(type_id_value);
                    handler();
                }
                catch (const std::out_of_range&)
                {
                    throw std::runtime_error{
                        "serialization::engine::deserialize : unknown type extracted : cx-hash=" +
                        std::to_string(type_id_value)};
                }
            }
            template <std::size_t count = 1>
            void deserialize_n()
            {
                for (std::size_t i{0}; i < count and not input_stream.eof(); ++i)
                {
                    deserialize();
                }
            }
            void deserialize_n(std::size_t count)
            {
                for (std::size_t i{0}; i < count and not input_stream.eof(); ++i)
                {
                    deserialize();
                }
            }
            void deserialize_all()
            {
                while (not input_stream.eof())
                {
                    deserialize();
                }
            }
        };
#if __clang__
        template <class on_deserialization_t, typename... Ts>
        in(std::istream&, on_deserialization_t&&, std::tuple<Ts...>) -> in<on_deserialization_t>;
#endif

        class out {
            std::ostream& output_stream;

          public:
            out(std::ostream& output)
                : output_stream{output}
            {}

            template <typename T>
            void serialize(T&& value) const
            {
                io_policy::write(output_stream, gcl::cx::typeinfo::hashcode<T>());
                io_policy::write(output_stream, std::forward<T>(value));
            }
            template <typename T>
            void serialize(const T& value) const
            {
                io_policy::write(output_stream, gcl::cx::typeinfo::hashcode<T>());
                io_policy::write(output_stream, std::forward<T>(value));
            }
            template <typename ...Ts>
            void serialize(signature<Ts...>&& argument) const
            {
                static_assert(((not std::is_reference_v<Ts>)&&...));
                static_assert(((not std::is_const_v<Ts>)&&...));

                using argument_storage_type = std::decay_t<std::remove_reference_t<decltype(argument)>>::storage_type;

                io_policy::write(output_stream, gcl::cx::typeinfo::hashcode<signature<Ts...>>());
                [&]<std::size_t... indexes>(std::index_sequence<indexes...>){
                    ((io_policy::write(output_stream, std::move(std::get<indexes>(argument.storage)))), ...);
                }(std::make_index_sequence<std::tuple_size_v<argument_storage_type>>{});
            }
            template <typename T>
            const out& operator<<(T&& value) const
            {
                serialize(std::forward<decltype(value)>(value));
                return *this;
            }
        };

        template <typename on_deserialization_t>
        using deserializer = in<on_deserialization_t>;
        using serializer = out;
    };
}

#include <sstream>

namespace gcl::io::tests::serialization
{
    static void test()
    {
        struct event_1 {};
        struct event_2 {};
        struct event_3 {};

        try
        {
            std::stringstream ss;

            using io_engine = typename gcl::io::serialization::engine<gcl::io::policy::binary>;
            {
                auto serializer = io_engine::out{ss};
                // serializer << event_1{} << event_2{} << event_3{};
                serializer << gcl::io::serialization::signature{event_1{}, event_2{}, event_3{}};
                serializer << gcl::io::serialization::signature{42, 'c'};
            }
            {
                using types = std::tuple<event_1, event_2, event_3>;

                int  call_counter{0};
                auto deserializer = io_engine::in{
                    ss,
                    gcl::functional::overload{
                        [&call_counter](event_1&&, event_2&&, event_3&&) mutable {
                            if (++call_counter not_eq 1)
                                throw std::runtime_error{
                                    "gcl::io::tests::serialization : overload signature : event : call order"};
                        },
                        [&call_counter](int&& i, char&& c) mutable {
                            if (++call_counter not_eq 2)
                                throw std::runtime_error{
                                    "gcl::io::tests::serialization : overload signature : pods : call order"};
                            if (i not_eq 42)
                                throw std::runtime_error{
                                    "gcl::io::tests::serialization : overload signature : pods : arg 1"};
                            if (c not_eq 'c')
                                throw std::runtime_error{
                                    "gcl::io::tests::serialization : overload signature : pods : arg 2"};
                        },
                    }};
                deserializer.deserialize_all();
            }
        }
        catch (const std::exception&)
        {
            throw;
        }
    }
}