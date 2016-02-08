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
	namespace Notification
	{
		struct OnDestructionINotify : public GCL::Notification::Notifiable<>
		{
			static const std::string DTOR_EVENT_NAME;

			~OnDestructionINotify()
			{
				this->TriggerEvent(DTOR_EVENT_NAME);
			}
		};
		const std::string OnDestructionINotify::DTOR_EVENT_NAME = "dtor";


		void	Make_OnDestructionINotify(GCL::Notification::Notifiable<> & obj)
		{
		}

	}
	namespace Container
	{
		struct SelfUpdate
		{
			using T_Element = Notification::OnDestructionINotify;
			using T_Container = std::set < T_Element * > ;	// Only ordered

			SelfUpdate() = default;

			inline size_t	size(void) const
			{
				return _content.size();
			}
			SelfUpdate &		operator+=(T_Element & elem)
			{
				std::pair<T_Container::iterator, bool> ret = _content.insert(&elem);

				if (ret.second == false)
					std::cerr << "[Warning] : Attempt to register an existing element" << std::endl;

				elem.on(Notification::OnDestructionINotify::DTOR_EVENT_NAME) += [this, &elem]()
				{
					this->remove(elem);
				};

				return *this;
			}
			/*template<template<typename T_Element> class T_ContainerOfElements>
			Container &		operator+=(T_ContainerOfElements & container)
			{
			for (auto & elem : container)
			*this += elem;
			}*/
			SelfUpdate & operator-=(T_Element & elem)
			{
				this->remove(elem);
				elem.on(Notification::OnDestructionINotify::DTOR_EVENT_NAME).clear();

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
			struct Toto : GCL::Notification::OnDestructionINotify
			{};

			static bool	Proceed(void)
			{
				try
				{
					SelfUpdate	intContainer;

					{
						std::vector<Toto>	totos(4);

						intContainer += totos.at(0);
						intContainer += totos.at(1);
						intContainer += totos.at(2);
						intContainer += totos.at(3);
						_GCL_DEBUG_INSTRUCTION(std::cout << "[+] Adding 4 elements     : " << intContainer.size() << std::endl);
						intContainer -= totos.at(1);
						_GCL_DEBUG_INSTRUCTION(std::cout << "[+] Removing 1 element    : " << intContainer.size() << std::endl);
					}
					_GCL_DEBUG_INSTRUCTION(std::cout << "[+] Destroying 4 elements : " << intContainer.size() << std::endl);
					return (intContainer.size() == 0);
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