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
// todo : detect if `on_deserialization_t::operator()` has multiples arguments

namespace gcl::io::serialization::type_traits
{
    template <class, class = void>
    struct has_decltype_deductible_parenthesis_op : std::false_type {};
    template <class T>
    struct has_decltype_deductible_parenthesis_op<T, std::void_t<decltype(&T::operator())>> : std::true_type {};
    template <class T>
    constexpr static auto has_decltype_deductible_parenthesis_op_v = has_decltype_deductible_parenthesis_op<T>::value;
}
namespace gcl::io::serialization
{
    template <typename... Ts>
    struct signature { // bind arguments together
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
            using type_id_t = gcl::cx::typeinfo::hashcode_t;
            using storage_type = std::unordered_map<type_id_t, std::function<void()>>;
            std::istream&        input_stream;
            on_deserialization_t on_deserialize;
            storage_type         storage;

            template <gcl::io::concepts::serializable... Ts>
            auto generate_type_handler()
            {
                static_assert(((not std::is_const_v<Ts>)&&...));
                static_assert(((not std::is_reference_v<Ts>)&&...));

                if constexpr (sizeof...(Ts) == 1)
                {
                    return std::pair{
                        gcl::cx::typeinfo::hashcode<Ts...>(), std::function<void()>{[this]() {
                            std::tuple_element_t<0, std::tuple<Ts...>> value;
                            io_policy::read(input_stream, value);
                            if (not input_stream.good())
                                throw std::runtime_error{
                                    "serialization::engine::deserialize (value) / storage::mapped::operator() "};
                            on_deserialize(std::move(value));
                        }}};
                }
                else
                {
                    using handler_signature = signature<Ts...>;
                    return std::pair{
                        gcl::cx::typeinfo::hashcode<handler_signature>(), std::function<void()>{[this]() {
                            std::tuple<Ts...> arguments;
                            const auto        read_one_argument = [this]<typename T>(T&& value) {
                                io_policy::read(input_stream, std::forward<T>(value));
                                if (not input_stream.good())
                                    throw std::runtime_error{
                                        "serialization::engine::deserialize (value) / "
                                        "storage::mapped::operator() "};
                            };
                            [&arguments, &read_one_argument ]<std::size_t... indexes>(std::index_sequence<indexes...>)
                            {
                                ((read_one_argument(std::move(std::get<indexes>(arguments)))), ...);
                            }
                            (std::make_index_sequence<std::tuple_size_v<decltype(arguments)>>{});

                            std::apply(on_deserialize, std::move(arguments));
                        }}};
                }
            }

          public:
            template <typename... serializable_types>
            in(std::istream& input, on_deserialization_t&& cb, std::tuple<serializable_types...>&& = std::tuple<>{})
                : input_stream{input}
                , on_deserialize{std::forward<decltype(cb)>(cb)}
                , storage{generate_type_handler<serializable_types>()...}
            {
                if constexpr (gcl::functional::type_traits::is_overload_v<on_deserialization_t>)
                {
                    const auto register_signature = [this]<typename... Ts>(std::tuple<Ts...>)
                    {
                        static_assert(sizeof...(Ts) not_eq 0);
                        static_assert(((not std::is_const_v<Ts>)&&...));
                        static_assert(((not std::is_reference_v<Ts>)&&...));

                        storage.insert(generate_type_handler<Ts...>());
                    };
                    // quick-fix : (see Clang frontend ICE hereunder)
                    // const auto register_deductible_signature = [&register_signature]<typename T>(T&&) {
                    const auto register_deductible_signature =
                        [&register_signature]<typename T>(mp::type_tag::type_to_type<T>&&) {
                            if constexpr (type_traits::has_decltype_deductible_parenthesis_op_v<T>)
                            {
                                using remove_cv_and_ref =
                                    gcl::mp::type_traits::merge_traits<std::remove_reference_t, std::decay_t>;
                                ((register_signature(
                                    gcl::mp::type_traits::transform_t<
                                        typename gcl::mp::function_traits_t<decltype(&T::operator())>::arguments,
                                        remove_cv_and_ref::type>{})));
                            }
                        };
                    [&register_deductible_signature]<typename... Ts>(gcl::functional::overload<Ts...> &&)
                    { // Clang frontend ICE : https://bugs.llvm.org/show_bug.cgi?id=49881
                        ((register_deductible_signature(mp::type_tag::type_to_type<Ts>{})),
                         ...); // quick-fix to Clang ICE
                        //((register_deductible_signature.template operator()<Ts>()), ...);
                    }
                    (std::forward<decltype(cb)>(cb));
                }
            }

            template <gcl::io::concepts::serializable... Ts>
            void register_types()
            { // todo : remove useless lambda
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
            template <std::size_t count_v = 1>
            void deserialize_n(std::size_t count = count_v)
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
        in(std::istream&, on_deserialization_t&&, std::tuple<Ts...>&&) -> in<on_deserialization_t>;
        template <class on_deserialization_t>
        in(std::istream&, on_deserialization_t&&) -> in<on_deserialization_t>;
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
            template <typename... Ts>
            void serialize(signature<Ts...>&& argument) const
            {
                static_assert(((not std::is_reference_v<Ts>)&&...));
                static_assert(((not std::is_const_v<Ts>)&&...));

                using argument_storage_type =
                    typename std::decay_t<std::remove_reference_t<decltype(argument)>>::storage_type;

                io_policy::write(output_stream, gcl::cx::typeinfo::hashcode<signature<Ts...>>());
                [&]<std::size_t... indexes>(std::index_sequence<indexes...>)
                {
                    ((io_policy::write(output_stream, std::move(std::get<indexes>(argument.storage)))), ...);
                }
                (std::make_index_sequence<std::tuple_size_v<argument_storage_type>>{});
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

#if defined(GCL_BUILD_RT_TESTS)
#include <sstream>
namespace gcl::io::tests::serialization
{
    static void test()
    {
        struct event_1 {};
        struct event_2 {};
        struct event_3 {};
        struct event_4 {};
        struct event_5 {};
        struct event_6 {};

        try
        {
            std::stringstream ss;

            using io_engine = typename gcl::io::serialization::engine<gcl::io::policy::binary>;
            {
                auto serializer = io_engine::out{ss};
                serializer << gcl::io::serialization::signature{event_1{}, event_2{}, event_3{}};
                serializer << gcl::io::serialization::signature{42, 'c'};
            }

            int call_counter{0};
            {
                auto deserializer = io_engine::in{
                    static_cast<std::istream&>(ss),
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
                if (call_counter not_eq 2 or not ss.eof())
                    throw std::runtime_error{
                        "gcl::io::tests::serialization : overload signature : incomplete extraction"};
            }
            {
                ss.clear();
                auto serializer = io_engine::out{ss};
                serializer << event_4{} << event_5{} << event_6{};
                ss.seekg(0);
            }

            {
                int call_counter{0};
                using types = std::tuple<event_4, event_5, event_6>;
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
                        [&call_counter](auto&& arg) mutable {
                            switch (++call_counter)
                            {
                            case 3:
                                if (not std::is_same_v<std::remove_reference_t<decltype(arg)>, event_4>)
                                    throw std::runtime_error{
                                        "gcl::io::tests::serialization : overload signature : auto : event_4"};
                                break;
                            case 4:
                                if (not std::is_same_v<std::remove_reference_t<decltype(arg)>, event_5>)
                                    throw std::runtime_error{
                                        "gcl::io::tests::serialization : overload signature : auto : event_5"};
                                break;
                            case 5:
                                if (not std::is_same_v<std::remove_reference_t<decltype(arg)>, event_6>)
                                    throw std::runtime_error{
                                        "gcl::io::tests::serialization : overload signature : auto : event_6"};
                                break;

                            default:
                                throw std::runtime_error{
                                    "gcl::io::tests::serialization : overload signature : auto : call_order"};
                            }
                        }},
                    types{}};

                deserializer.register_types<event_1, event_2, event_3>();
                deserializer.deserialize_n<3>(); // deserialize 3 elements
                deserializer.deserialize_n(2);   // deserialize 2 elements
                deserializer.deserialize_all();  // deserialize 0 elements (no more remaining)
                if (call_counter not_eq 5 or not ss.eof())
                    throw std::runtime_error{
                        "gcl::io::tests::serialization : overload signature + types  : incomplete extraction"};
            }
        }
        catch (const std::exception&)
        {
            throw;
        }
    }
}
#endif