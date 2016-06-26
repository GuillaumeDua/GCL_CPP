#ifndef GCL_TEST_UTILS__
# define GCL_TEST_UTILS__

#include <iostream>
#include <vector>
#include <list>
#include <cassert>
#include <unordered_map>
#include <functional>
#include <string>
#include <chrono>
#include <thread>

namespace GCL
{
	namespace Experimental
	{
		namespace TestUtils
		{

			struct RT_Exception_controler
			{
				struct Observer
				{
					virtual void	NotifyPropertyChanged(void) = 0;
				};

				using T_KeyHash = std::string;	// todo : key_type, mapped_type, iterator : STL compatibility
				struct T_HookPoint
				{
					using T_Callback = std::function<void()>;

					T_HookPoint(T_Callback && cb)
						: _conditionnalSnippet(cb)
					{}

					T_Callback	_conditionnalSnippet = nullptr;
					bool isActive = false;
					// todo : std::string description : callback code to string
				};
				using T_ObserversList = std::list<std::shared_ptr<Observer>>;

			private:
				using T_Container = std::unordered_map<T_KeyHash, T_HookPoint>;
				static T_Container _container;
				static T_ObserversList	_observers;

			public:

				static void								OnPropertyChanged(void)
				{
					for (auto & obs : _observers)
						obs->NotifyPropertyChanged();
				}
				static const T_Container::mapped_type &	Insert(T_Container::value_type && value)
				{
					T_Container::iterator it = _container.find(value.first);
					if (it == _container.end())
					{
						auto insert_ret = _container.emplace(value);
						assert(insert_ret.second);
						it = insert_ret.first;
						OnPropertyChanged();
					}
					return it->second;
				}

				static inline void	Activate(const size_t distance)
				{
					T_Container::iterator it = _container.begin();
					std::advance(it, distance);
					Activate(it);
				}
				static inline void	Activate(const T_KeyHash key)
				{
					Activate(_container.find(key));
				}
				static inline void	Activate(T_Container::iterator & it)
				{
					it->second.isActive = true;
					std::cout << "[+]::[INFO] : Activating hookpoint at : " << it->first << std::endl;
				}
			};

#define CONDITIONAL_SNIPPET(expr)	static const GCL::Experimental::TestUtils::RT_Exception_controler::T_KeyHash key = std::string(__FILE__ " line ") + std::to_string(__LINE__);																																										\
									const GCL::Experimental::TestUtils::RT_Exception_controler::T_HookPoint & hook =  GCL::Experimental::TestUtils::RT_Exception_controler::Insert(std::pair<GCL::Experimental::TestUtils::RT_Exception_controler::T_KeyHash, GCL::Experimental::TestUtils::RT_Exception_controler::T_HookPoint>(key, [&]() { expr }));	\
									if (hook.isActive)																																																																									\
										hook._conditionnalSnippet();
			struct Test
			{
				static bool Proceed(void)
				{
					for (size_t i = 0; i < 2; ++i)
					{
						std::cout << "[+]::[TEST] : " << i << " : ";
						try
						{
							CONDITIONAL_SNIPPET(throw std::runtime_error("std::runtime_error"););
							std::cout << "No throw" << std::endl;
						}
						catch (const std::exception & ex)
						{
							std::cout << "Exception catch : " << ex.what() << std::endl;
						}
						std::this_thread::sleep_for(std::chrono::seconds(1));

						RT_Exception_controler::Activate(0);					// simulate activation at index 0
					}
					return true;
				}
			};
		}
	}
}
//#define CONDITIONAL_SNIPPET(expr)	static const RT_Exception_controler::T_KeyHash key = std::string(__FILE__ " line ") + std::to_string(__LINE__);	\
//									RT_Exception_controler::T_Container::iterator it = RT_Exception_controler::_container.find(key);				\
//									if (it == RT_Exception_controler::_container.cend())															\
//									RT_Exception_controler::_container.insert(std::make_pair(key, std::move([&]() { expr })));						\
//									else if (it->second.isActive)																					\
//									it->second._conditionnalSnippet();



#endif // GCL_TEST_UTILS__