#ifndef GCL_OLD__SELF_UPDATE_HPP__
# define GCL_OLD__SELF_UPDATE_HPP__

# error this file is deprecated (too old / unstable) and will be delete shortly

# include <set>
# include <vector>
# include <functional>
# include <iostream>
# include <algorithm>

# include <gcl_cpp/old/EventHandler.h>
# include <gcl_cpp/preprocessor.hpp>
# include <gcl_cpp/functional.hpp>

namespace gcl::old
{
	// todo : replace by new gcl::event
	namespace Event
	{
		static const std::string DTOR_EVENT_NAME = "_dtor_";
	}
	namespace Container
	{
		template <typename T_Element>
		struct SelfUpdate
		{
			using T_Container = std::set < T_Element * >;	// Only ordered

			SelfUpdate() = default;
			/*SelfUpdate(std::initializer_list<T_Element> initList)
			{}*/

			inline size_t	size(void) const
			{
				return _content.size();
			}
			SelfUpdate &	operator+=(T_Element & elem)
			{
				std::pair<T_Container::iterator, bool> ret = _content.insert(&elem);

				if (ret.second == false)
					std::cerr << "[Warning] : Attempt to register an existing element" << std::endl;

				// Note : elem is **(ret.first)
				elem.on(Event::DTOR_EVENT_NAME).emplace_back(std::move(gcl::functional::finally([this, &elem]()
				{
					this->remove(elem);
				})));

				return *this;
			}
			/*template<template<typename T_Element> class T_ContainerOfElements>	// [Todo] : Wait for concept/contraint to use `requires` here
			Container &		operator+=(T_ContainerOfElements & container)
			{
			for (auto & elem : container)
			*this += elem;
			}*/
			SelfUpdate &	operator-=(T_Element & elem)
			{
				this->remove(elem);
				elem.on(Notification::DTOR_EVENT_NAME).clear();

				return *this;
			}

		protected:
			SelfUpdate &	remove(T_Element & elem)
			{
				T_Container::iterator	matchIt = _content.find(&elem);
				if (matchIt == _content.cend())
					throw std::runtime_error("Attempt to remove an unregistered element");

				_content.erase(matchIt);
				return *this;
			}

			T_Container	_content;
		};

		struct Test
		{
			static bool	Proceed(void)
			{
				try
				{
					bool was_cb_called(false);
					{
						gcl::old::events::EventHandler<> eventHandler;
						{
							eventHandler.on(Event::DTOR_EVENT_NAME).emplace_back(std::move(gcl::functional::finally([&was_cb_called]() { was_cb_called = true; })));
						}
						if (was_cb_called)
							return false;
					}

					struct Toto : public gcl::old::events::EventHandler<>
					{};

					bool wasElementCorrectlyAdded(false);
					SelfUpdate<Toto> container;
					{
						Toto toto1, toto2;

						container += toto1;
						container += std::move(toto2);

						wasElementCorrectlyAdded = (container.size() == 2);
					}

					return was_cb_called && wasElementCorrectlyAdded && container.size() == 0;
				}
				catch (const std::exception & ex)
				{
					std::cerr << "[Error] : STD exception catch : [" << ex.what() << ']' << std::endl;
				}
				catch (...)
				{
					std::cerr << "[Error] : Unknown exception catch" << std::endl;
				}
				return false;
			}
		};
	}
}
#endif // GCL_OLD__SELF_UPDATE_HPP__