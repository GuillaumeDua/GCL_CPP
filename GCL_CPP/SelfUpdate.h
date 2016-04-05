#ifndef __GCL__SELF_UPDATE__
# define __GCL__SELF_UPDATE__

# include <set>
# include <vector>
# include <functional>
# include <iostream>
# include <algorithm>

# include "Notification.h"
# include "Preprocessor.h"

namespace GCL
{
	struct OnDestructionCalledStdFunction : public std::function<void()>
	{
		explicit OnDestructionCalledStdFunction(std::function<void()> && func)
		{
			func.swap(*this);
		}
		OnDestructionCalledStdFunction(const OnDestructionCalledStdFunction & w)
		{
			const_cast<OnDestructionCalledStdFunction &>(w).swap(*this);
		}
		~OnDestructionCalledStdFunction()
		{
			if (*this)
				(*this)();
		}
	};

	namespace Notification
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
				elem.on(Notification::DTOR_EVENT_NAME).emplace_back(std::move(OnDestructionCalledStdFunction([this, &elem]()
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
						GCL::Notification::EventHandler<> notifiable;
						{
							notifiable.on(Notification::DTOR_EVENT_NAME).emplace_back(std::move(GCL::OnDestructionCalledStdFunction([&was_cb_called](){ was_cb_called = true; })));
						}
						if (was_cb_called)
							return false;
					}

					struct Toto : public Notification::EventHandler<>
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
					std::cerr << "[Error] : STD Exception catch : [" << ex.what() << ']' << std::endl;
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
#endif // __GCL__SELF_UPDATE__