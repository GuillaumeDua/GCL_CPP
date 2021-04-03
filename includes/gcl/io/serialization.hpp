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
                    auto register_each_tuple_element = [this]<typename... Ts>(std::tuple<Ts...>)
                    {
                        static_assert(((not std::is_const_v<Ts>)&&...));
                        static_assert(((not std::is_reference_v<Ts>)&&...));

                        register_types<Ts...>();
                    };
                    [register_each_tuple_element]<typename... Ts>(gcl::functional::overload<Ts...> &&)
                    {
                        using remove_cv_and_ref = gcl::mp::type_traits::merge_traits<std::remove_reference_t, std::decay_t>;

                        ((register_each_tuple_element(gcl::mp::type_traits::transform_t<
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
                (storage.insert(generate_type_handler<Ts>()), ...);
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
                serializer << event_1{} << event_2{} << event_3{};
            }
            {
                using types = std::tuple<event_1, event_2, event_3>;

                int  call_counter{0};
                auto deserializer = io_engine::in{
                    ss,
                    gcl::functional::overload{
                        [&call_counter](event_1&&) mutable {
                            if (++call_counter != 1)
                                throw std::runtime_error{"gcl::io::tests::serialization::test : event_1"};
                        },
                        [&call_counter](event_2&&) mutable {
                            if (++call_counter != 2)
                                throw std::runtime_error{"gcl::io::tests::serialization::test : event_2"};
                        },
                        [&call_counter](event_3&&) mutable {
                            if (++call_counter != 3)
                                throw std::runtime_error{"gcl::io::tests::serialization::test : event_3"};
                        },
                        /*[&call_counter]<typename T>(T&& arg) mutable {
                            static_assert(not std::is_same_v<decltype(arg), event_3>);

                            if constexpr (std::is_same_v<T, event_1>)
                                if (++call_counter not_eq 1)
                                    throw std::runtime_error{"gcl::io::tests::serialization::test : event_1"};
                            if constexpr (std::is_same_v<T, event_2>)
                                if (++call_counter not_eq 2)
                                    throw std::runtime_error{"gcl::io::tests::serialization::test : event_2"};
                        }*/}
                };
                deserializer.deserialize_all();
            }
        }
        catch (const std::exception& error)
        {
            std::cerr << "error : " << error.what() << '\n';
        }
    }
}