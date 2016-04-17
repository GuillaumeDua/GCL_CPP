#ifndef GCL_TMP_H_
# define GCL_TMP_H_

namespace GCL
{
    namespace TMP
    {
        struct NullType {};

        template <bool condition, typename _THEN, typename _ELSE>
        struct IF
        {};
        template <typename _THEN, typename _ELSE>
        struct IF<true, _THEN, _ELSE>
        {
            using _Type = _THEN;
        };
        template <typename _THEN, typename _ELSE>
        struct IF<false, _THEN, _ELSE>
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
