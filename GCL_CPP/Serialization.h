#ifndef GCL_SERIALIZATION__
# define GCL_SERIALIZATION__

# include <sstream>
# include <map>
# include "TypeTraits.h"
# include "IO.h"
# include "TemplateMetaProgramming.h"

namespace GCL
{
	namespace Serialization
	{
		struct Interface
		{
			virtual ~Interface() {};

			virtual std::istream & operator>>(std::istream &) = 0;
			virtual std::ostream & operator<<(std::ostream &) const = 0;

			friend inline std::ostream& operator<<(std::ostream & os, const Interface & s)
			{
				s.operator << (os);
				return os;
			}
			friend inline std::istream& operator>>(std::istream & is, Interface & s)
			{
				s.operator >> (is);
				return is;
			}
		};

		template
		<
				typename IO_Policy
			,	typename T_SerializableInterface
			,	typename ... T_SerializableTypes
		>
		struct Serializer
		{
			using T_IOPolicy	= IO_Policy;
			using T_TypeManager = typename TypeTrait::InterfaceIs<T_SerializableInterface>::template OfTypes<T_SerializableTypes...>;
			using _index_type	= typename TypeTrait::InterfaceIs<T_SerializableInterface>::index_type;

			// static typename T_TypeManager::Indexer _typeIndex;

			template <typename T>
			static void							write(std::ostream & os, const T & var)
			{
				T_IOPolicy::write(os, T_TypeManager::T_TypePack::template indexOf<T>()); // throw out_of_range if T is not part of T_SerializableTypes
				// T_IOPolicy::write(os, _typeIndex.indexOf<T>()); 
				os << var;
			}
			static T_SerializableInterface *	read(std::istream & is)
			{
				_index_type typeId;
				T_IOPolicy::read(is, typeId);
				auto & constructor = T_TypeManager::index.at(typeId);
				T_SerializableInterface * obj = constructor();
				is >> *obj;
				return obj;
			}
		};

		struct Test
		{
			struct Toto : Serialization::Interface
			{
				int _i;
				~Toto() override {}
				std::istream & operator >> (std::istream & is) override
				{
					IO::Policy::Binary::read(is, _i);
					return is;
				}
				std::ostream & operator << (std::ostream & os) const override
				{
					IO::Policy::Binary::write(os, _i);
					return os;
				}
			};
			struct Titi : Serialization::Interface
			{
				std::string _str;
				~Titi() override {}
				std::istream & operator >> (std::istream & is) override
				{
					IO::Policy::Binary::read(is, _str);
					return is;
				}
				std::ostream & operator << (std::ostream & os) const override
				{
					IO::Policy::Binary::write(os, _str);
					return os;
				}
			};

			using _Serializer = Serializer
			<
				// IO Policy
					IO::Policy::Binary
				// Serializable interface
				,	Serialization::Interface
				// Serializable types
				,	Toto
				,	Titi
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

				Serialization::Interface * w_titi1 = _Serializer::read(ss);
				Serialization::Interface * w_toto1 = _Serializer::read(ss);
				Serialization::Interface * w_titi2 = _Serializer::read(ss);
				Serialization::Interface * w_toto2 = _Serializer::read(ss);

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
