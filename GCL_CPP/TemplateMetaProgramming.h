#ifndef GCL_TMP_H_
# define GCL_TMP_H_

namespace GCL
{
    namespace TMP
    {
        struct NullType {};

		template <bool condition, typename _THEN, typename _ELSE>	struct IF
        {};
		template <typename _THEN, typename _ELSE>					struct IF<true, _THEN, _ELSE>
        {
            using _Type = _THEN;
        };
		template <typename _THEN, typename _ELSE>					struct IF<false, _THEN, _ELSE>
        {
            using _Type = _ELSE;
        };

        template <size_t _pID = 0>
        struct List
        {
            static constexpr size_t _ID = _pID;

            using _ThisType = List<_ID>;
            using _Next = List<_ID + 1>;
            using _PREV = TMP::IF<(_ID == 0), TMP::NullType, List<_ID - 1> >;
        };

		// [Todo] : Refactoring
        template <typename Type, template <typename> class Visitor, size_t _ItMax, size_t _It = 0, bool Continue = true>
        struct For
        {
            static void    iterate()
            {
                Visitor<Type>::visit();
                For<Type::_Next, Visitor, _ItMax, _It + 1, (_It < _ItMax)>::iterate();
            }

        };
        template <typename Type, template <typename> class Visitor, size_t _ItMax, size_t _It>
        struct For<Type, Visitor, _ItMax, _It, false>
        {
            static void    iterate() {}
        };

		template <typename ... T>				struct Foreach;
		template <typename T0, typename ... T>	struct Foreach<T0, T...>
		{
			Foreach() = delete;
			Foreach(const Foreach &) = delete;
			Foreach(const Foreach &&) = delete;

			template <template <typename> class T_Functor>
			static void	Call(void)
			{
				T_Functor<T0>::Call();
				Foreach<T...>::Call<T_Functor>();
			}
			template <template <typename> class T_Functor, size_t N = 0>
			static void	CallAt(const size_t pos)
			{
				if (N == pos)	T_Functor<T0>::Call();
				else			Foreach<T...>::CallAt<T_Functor, (N + 1)>(pos);
			}
			template <template <typename> class T_Functor, size_t N = 0>
			static typename T_Functor<T0>::return_type	CallAt_withReturn(const size_t pos)
			{
				if (N == pos)	return T_Functor<T0>::Call();
				else			return Foreach<T...>::CallAt_withReturn<T_Functor, (N + 1)>(pos);
			}
		};
		template <>								struct Foreach<>
		{
			Foreach() = delete;
			Foreach(const Foreach &) = delete;
			Foreach(const Foreach &&) = delete;

			template <template <typename> class T_Functor>
			static void	Call(void)
			{}
			template <template <typename> class T_Functor, size_t N = 0>
			static void	CallAt(const size_t pos)
			{
				throw std::out_of_range("template <typename ... T> struct Foreach::CallAt");
			}
			template <template <typename> class T_Functor, size_t N = 0>
			static typename T_Functor<void>::return_type	CallAt_withReturn(const size_t pos)
			{
				throw std::out_of_range("template <typename ... T> struct Foreach::CallAt_withReturn");
			}
		};

		template <typename ...T>
		struct TypeContainer
		{
			using _Types = std::tuple<T...>;

			template <std::size_t N>
			using TypeAt = typename std::tuple_element<N, _Types>::type;
			// using TypeAt = std::remove_reference<decltype(std::get<N>(tuple))>::type
			template <typename T_Search>
			constexpr static const size_t IndexOf(void)
			{
				return GCL::TypeTrait::IndexOf< T_Search, _Types>::value;
			}
		};

        struct Test
        {
            template <typename T>
            struct Visitor_ListIdPrinter
            {
                static void visit()
                {
                    std::cout << T::_ID << std::endl;
                }
            };

            static bool    Proceed()
            {
                std::cout << "Test" << std::endl;
                TMP::For<TMP::List<>, Visitor_ListIdPrinter, 10>::iterate();
                return true;
            }
        };
    }
}

#endif // GCL_TMP_H_
