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
#include <mutex>

namespace GCL
{
	namespace Experimental
	{
		namespace TestUtils
		{
			namespace Inline
			{
				struct RT_Exception_controler final
				{
					using key_type = std::string;
					using mapped_type = bool;
					using value_type = std::pair<const key_type, mapped_type>;
					
					struct Observer
					{
						virtual void	NotifyPropertyChanged(value_type &) = 0;
					};
					using T_ObserversList = std::list<std::shared_ptr<Observer>>;

				private:
					using T_Container = std::unordered_map<key_type, mapped_type>;
					static T_Container		_container;
					static std::mutex		_container_mutex;
					static T_ObserversList	_observers;
					static std::mutex		_obs_mutex;
					static volatile bool	_isActive;
					static void								OnPropertyChanged(value_type & property)
					{
						std::async(std::launch::async, [&property]()
						{
							std::lock_guard<std::mutex> lock(_obs_mutex);
							for (auto & obs : _observers)
								obs->NotifyPropertyChanged(property);
						});
					}

				public:

					static inline volatile bool &			IsActive(void)
					{
						return _isActive;
					}
					static void								AddObserver(std::shared_ptr<Observer> & obs)
					{
						std::lock_guard<std::mutex> lock(_obs_mutex);
						_observers.push_back(obs);
					}
					static T_Container::mapped_type			Get(const T_Container::key_type & key)
					{
						std::lock_guard<std::mutex>	lock(_container_mutex);
						T_Container::iterator it = _container.find(key);
						if (it == _container.end())
						{
							auto insert_ret = _container.emplace(value_type(key, false));
							assert(insert_ret.second);
							OnPropertyChanged(*(insert_ret.first));	// costly
						}
						return _container.at(key);
					}

					static inline void						Activate(const size_t distance)
					{
						std::lock_guard<std::mutex>	lock(_container_mutex);
						T_Container::iterator it = _container.begin();
						std::advance(it, distance);
						it->second = true;
						std::cout << "[+]::[INFO] : Activating hookpoint at : " << it->first << std::endl;
					}
					static inline void						Activate(const key_type key)
					{
						std::lock_guard<std::mutex>	lock(_container_mutex);
						T_Container::iterator it = _container.find(key);
						assert(it != _container.end());
						it->second = true;
					}
				};

#define CONDITIONAL_SCOPE_INLINE(expr)		static const GCL::Experimental::TestUtils::Inline::RT_Exception_controler::key_type key = std::string(__FILE__ " line ") + std::to_string(__LINE__);													\
											GCL::Experimental::TestUtils::Inline::RT_Exception_controler::mapped_type hook_active = RT_Exception_controler::IsActive() && GCL::Experimental::TestUtils::Inline::RT_Exception_controler::Get(key);	\
											if (hook_active)																																														\
											{																																																		\
												expr																																																\
											}

				struct Test
				{
					struct LogObserver : RT_Exception_controler::Observer
					{
						void	NotifyPropertyChanged(RT_Exception_controler::value_type & elem) override
						{
							std::cout << "[+]::[TEST] : New entry at : " << elem.first << std::endl;
							elem.second = true;	// activate the element
							std::cout << "[+]::[TEST] : New entry activated" << std::endl;
						}
					};

					static bool Proceed(void)
					{
						std::shared_ptr<RT_Exception_controler::Observer> logObs = std::make_shared<LogObserver>();

						RT_Exception_controler::AddObserver(logObs);

						/*RT_Exception_controler::IsActive() = true;*/

						for (size_t i = 0; i < 2; ++i)
						{
							try
							{
								CONDITIONAL_SCOPE_INLINE(throw std::runtime_error("std::runtime_error"););
								std::cout << "[+]::[TEST] : " << i << " : No throw" << std::endl;
							}
							catch (const std::exception & ex)
							{
								std::cout << "[+]::[TEST] : " << i << " : Exception catch : " << ex.what() << std::endl;
							}

							// RT_Exception_controler::Activate(0);					// simulate activation at index 0
						}
						return true;
					}
				};
			}
		}
	}
}


#endif // GCL_TEST_UTILS__
