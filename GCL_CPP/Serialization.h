#ifndef GCL_SERIALIZATION__
# define GCL_SERIALIZATION__

# include "TypeTraits.h"
# include "IO.h"
# include "TemplateMetaProgramming.h"
# include "Introspection.hpp"

# include <sstream>
# include <map>
# include <queue>
# include <memory>

GCL_Introspection__GenHasNested(NotSerializable);

namespace GCL
{
	namespace Serialization
	{
		template <class T_Interface>
		struct InterfaceIs
		{
			using _InterfaceType = T_Interface;

			template <typename ...Types>
			struct OfTypes
			{
				// TODO : (void)GCL::TMP::Foreach<Types...>::template Require<IsChildOf<T_Interface>>

				using _TypesPack = typename GCL::TypeTrait::TypePack<Types...>;
				using _ThisType = typename OfTypes<Types...>;

				template <typename T_IO_POlicy = GCL::IO::Policy::Binary>
				struct Writer
				{
					explicit	Writer(std::ostream & oStream)
						: _oStream(oStream)
					{}
					template <typename T>
					Writer &	operator<<(const T & element)
					{
						write(_oStream, element);
						return *this;
					}
					template <typename T>
					static constexpr void	write(std::ostream & os, const T & var)
					{
						static_assert(std::is_base_of<_InterfaceType, T>::value, "GCL::Serialization::InterfaceIs<I>::Writer::write<T> : T is not child of I");
						WriterImpl<T>::write_impl<>(os, var);
					}

				private:
					template <typename T>
					struct WriterImpl
					{
						static constexpr bool isSerializable = !GCL::Introspection::has_NotSerializable_nested<T>::value;

						template <bool _isSerializable = isSerializable>
						static constexpr inline void	write_impl(std::ostream & os, const T & var);
						template <>
						static constexpr inline void	write_impl<true>(std::ostream & os, const T & var)
						{
							T_IO_POlicy::write(os, _TypesPack::template indexOf<T>());
							os << var; // WTF this is constexpr ?
						}
						template <>
						static constexpr inline void	write_impl<false>(std::ostream & os, const T & var)
						{
							static_assert(false, "This type has \"NeverSerialize\"");
						}
					};

					std::ostream & _oStream;
				};
				template <typename T_IO_POlicy = GCL::IO::Policy::Binary>
				struct Reader
				{
					using T_TypeManager = typename TypeTrait::InterfaceIs<_InterfaceType>::template OfTypes<Types...>;

					explicit				Reader(std::istream & istream)
						: _iStream(istream)
					{}

					inline operator bool() const
					{
						return _iStream.operator bool();
					}

					template <typename T>
					static void				read(std::istream & is, T & var)
					{
						size_t typeIndex;
						is >> typeIndex;
						_GCL_ASSERT(_TypesPack::template indexOf<T>() == typeIndex);
						is >> var;
					}
					static std::unique_ptr<_InterfaceType> read(std::istream & is)
					{
						_GCL_ASSERT(is);
						size_t typeIndex;

						T_IO_POlicy::read(is, typeIndex);
						if (is.eof()) return 0x0;

						auto & constructor = T_TypeManager::index.at(typeIndex).defaultConstructeurCallerOp;
						std::unique_ptr<_InterfaceType> elem(constructor());
						is >> *elem;
						return elem;
					}

					Reader &				operator >> (std::unique_ptr<_InterfaceType> & element)
					{
						_GCL_ASSERT(_iStream);
						element = read(_iStream);
						return *this;
					}
					Reader &				operator >> (std::queue<std::unique_ptr<_InterfaceType>> & elemQueue)
					{
						_GCL_ASSERT(_iStream);
						std::unique_ptr<_InterfaceType> elem = nullptr;
						while (*this >> elem)
							elemQueue.emplace(std::move(elem));
						return *this;
					}

				private:
					std::istream & _iStream;
				};
			};
		};

#define GenTestClass(name, type)																			\
	struct name																								\
		: TestInterface																						\
	{																										\
			name() = default;																				\
			name(type value)																				\
			: _value(value)																					\
			{}																								\
			type _value;																					\
																											\
			void	DoStuff(void) const override { std::cout << ""#name##"" " -> value=[" << _value << ']' << std::endl;}\
			std::ostream & operator<<(std::ostream & os) const  override	{ GCL::IO::Policy::Binary::write(os, _value); return os; }	\
			std::istream & operator>>(std::istream & is)		override	{ GCL::IO::Policy::Binary::read (is, _value); return is; }	\
	};

		struct TestInterface
		{
			virtual void DoStuff() const = 0;

			virtual std::istream & operator >> (std::istream &) = 0;
			virtual std::ostream & operator<<(std::ostream &) const = 0;

			friend inline std::ostream& operator<<(std::ostream & os, const TestInterface & s)
			{
				s.operator << (os);
				return os;
			}
			friend inline std::istream& operator >> (std::istream & is, TestInterface & s)
			{
				s.operator >> (is);
				return is;
			}
		};

		GenTestClass(Toto, int)
		GenTestClass(Titi, std::string)
		GenTestClass(Tata, int)
		GenTestClass(Tutu, std::string)

		struct Test
		{
			static bool Proceed(void)
			{
				using Writer = Serialization::InterfaceIs<TestInterface>::OfTypes<Toto, Titi, Tata, Tutu>::Writer<>;

				std::stringstream ss;

				Writer writer(ss);
				writer
					<< Toto{ 42 }
					<< Titi{ "Hello, world" }
					<< Tata{ 130390 }
					<< Tutu{ "Morning' coffee" }
				;
				std::cout << "Serialized : [" << ss.str() << ']' << std::endl;

				using Reader = Serialization::InterfaceIs<TestInterface>::OfTypes<Toto, Titi, Tata, Tutu>::Reader<>;

				try
				{

					Reader			reader(ss);

					std::queue<std::unique_ptr<TestInterface>> elements;
					reader
						>> elements
						;

					while (not elements.empty())
					{
						elements.front()->DoStuff();
						elements.pop();
					}
				}
				catch (const std::exception & ex)
				{
					std::cerr << ex.what() << std::endl;
				}

				return true;
			}
		};
	}

	namespace OLD
	{
		namespace Serialization
		{
			struct Interface
			{
				virtual ~Interface() {};

				virtual std::istream & operator >> (std::istream &) = 0;
				virtual std::ostream & operator<<(std::ostream &) const = 0;

				friend inline std::ostream& operator<<(std::ostream & os, const Interface & s)
				{
					s.operator << (os);
					return os;
				}
				friend inline std::istream& operator >> (std::istream & is, Interface & s)
				{
					s.operator >> (is);
					return is;
				}
			};

			template
				<
				typename IO_Policy
				, typename T_SerializableInterface
				, typename ... T_SerializableTypes
				>
				struct Serializer
			{
				using T_IOPolicy = IO_Policy;
				using T_TypeManager = typename GCL::TypeTrait::InterfaceIs<T_SerializableInterface>::template OfTypes<T_SerializableTypes...>;
				using _index_type = typename GCL::TypeTrait::InterfaceIs<T_SerializableInterface>::index_type;

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
					auto & constructor = T_TypeManager::index.at(typeId).defaultConstructeurCallerOp;
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
					, Serialization::Interface
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
}
#endif // GCL_SERIALIZATION__
