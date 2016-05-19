#ifndef GCL_SERIALIZATION__
# define GCL_SERIALIZATION__

# include <sstream>
# include "IO.h"
# include "TemplateMetaProgramming.h"

namespace GCL
{
	namespace Serialization
	{
		struct Serializable
		{
			template <typename T>
			struct CtorCaller
			{
				using return_type = Serializable*;
				static return_type	Call(void)
				{
					return reinterpret_cast<return_type>(new T);
				}
			};

			void read(std::istream & is)
			{
				is >> *this;
			}
			void write(std::ostream & os)
			{
				os << *this;
			}
			virtual ~Serializable() {}
			virtual std::istream & operator >> (std::istream &) = 0;
			virtual std::ostream & operator << (std::ostream &) const = 0;
			friend std::ostream& operator<<(std::ostream & os, const Serializable & s)
			{
				s.operator << (os);
				return os;
			}
			friend std::istream& operator >> (std::istream & is, Serializable & s)
			{
				s.operator >> (is);
				return is;
			}
		};
		template
			<
			typename IO_Policy
			, typename ...T_SerializableTypes
			>
			struct Serializer
		{
			using T_IOPolicy = IO_Policy;
			using _Types = typename TypeContainer<T_SerializableTypes...>;
			template <typename T> // require : Serializable
			static void				write(std::ostream & os, const T & var)
			{
				T_IOPolicy::write(os, _Types::IndexOf<T>());
				os << var;
			}
			static Serializable*	read(std::istream & is)
			{
				_Types::Index_type typeId;
				T_IOPolicy::read(is, typeId);
				Serializable * obj = Foreach<T_SerializableTypes...>::CallAt_withReturn<Serializable::CtorCaller>(typeId);
				obj->read(is); // // *obj << is;
				return obj;
			}
		};

		struct Test
		{
			struct Toto : Serializable
			{
				int _i;
				~Toto() {}
				std::istream & operator >> (std::istream & is)
				{
					IO::Policy::Binary::read(is, _i);
					return is;
				}
				std::ostream & operator << (std::ostream & os) const
				{
					IO::Policy::Binary::write(os, _i);
					return os;
				}
			};
			struct Titi : Serializable
			{
				std::string _str;
				~Titi() {}
				std::istream & operator >> (std::istream & is)
				{
					IO::Policy::Binary::read(is, _str);
					return is;
				}
				std::ostream & operator << (std::ostream & os) const
				{
					IO::Policy::Binary::write(os, _str);
					return os;
				}
			};

			using _Serializer = Serializer
				<
				// IO Policy
				IO::Policy::Binary
				// Serializable types
				, Toto
				, Titi
				>;

			static bool	Proceed(void)
			{
				std::stringstream ss;
				Titi r_titi1; r_titi1._str = "titi1_str";
				Titi r_titi2; r_titi2._str = "this_is_titi2_str";
				Toto r_toto1; r_toto1._i = 42;
				Toto r_toto2; r_toto2._i = 11223344;

				_Serializer::write(ss, r_titi1);
				_Serializer::write(ss, r_toto1);
				_Serializer::write(ss, r_titi2);
				_Serializer::write(ss, r_toto2);

				Serializable * w_titi1 = _Serializer::read(ss);
				Serializable * w_toto1 = _Serializer::read(ss);
				Serializable * w_titi2 = _Serializer::read(ss);
				Serializable * w_toto2 = _Serializer::read(ss);

				std::cout << "DEBUG : " << std::endl;
				std::cout << reinterpret_cast<Titi*>(w_titi1) << std::endl;
				std::cout << reinterpret_cast<Titi*>(w_titi1)->_str << std::endl;
				std::cout << "/DEBUG : " << std::endl;
				return (
					r_titi1._str == reinterpret_cast<Titi*>(w_titi1)->_str
					&& r_titi2._str == reinterpret_cast<Titi*>(w_titi2)->_str
					&& r_toto1._i == reinterpret_cast<Toto*>(w_toto1)->_i
					&& r_toto2._i == reinterpret_cast<Toto*>(w_toto2)->_i
					);
			}
		};
	}
}
#endif // GCL_SERIALIZATION__
