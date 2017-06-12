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

#include <gcl_cpp/functionnal.hpp>

namespace gcl
{
	namespace experimental
	{
		namespace test_utils
		{
			namespace Inline
			{
				struct RT_scope_controler final
				{
					using key_type = std::string;
					using mapped_type = bool;
					using value_type = std::pair<const key_type, mapped_type>;
					
					struct Observer
					{
						virtual void	NotifyPropertyChanged(value_type &) = 0;
					};
					using T_ObserversList = std::list<std::shared_ptr<Observer>>;

					using T_Container = std::unordered_map<key_type, mapped_type>;

				private:
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
					template <bool default_active_value = false>
					static T_Container::mapped_type			Get(const T_Container::key_type & key)
					{
						std::lock_guard<std::mutex>	lock(_container_mutex);
						T_Container::iterator it = _container.find(key);
						if (it == _container.end())
						{
							auto insert_ret = _container.emplace(value_type(key, default_active_value));
							assert(insert_ret.second);
							OnPropertyChanged(*(insert_ret.first));	// Can be costly
						}
						return _container.at(key);
					}

					static inline void						Activate(const size_t distance)
					{
						SetMappedValue(distance, true);
					}
					static inline void						Activate(const key_type key)
					{
						SetMappedValue(key, true);
					}
					static inline void						Desactivate(const size_t distance)
					{
						SetMappedValue(distance, false);
					}
					static inline void						Desactivate(const key_type key)
					{
						SetMappedValue(key, false);
					}

				protected:
					static inline void						SetMappedValue(const size_t index, T_Container::mapped_type new_value)
					{
						std::lock_guard<std::mutex>	lock(_container_mutex);
						T_Container::iterator it = _container.begin();
						std::advance(it, index);
						it->second = new_value;
					}
					static inline void						SetMappedValue(const key_type & key, T_Container::mapped_type new_value)
					{
						std::lock_guard<std::mutex>	lock(_container_mutex);
						T_Container::iterator it = _container.find(key);
						assert(it != _container.end());
						it->second = new_value;
					}
				};

#define CONDITIONAL_SCOPE_INLINE(expr)	{																																																			\
											static const gcl::experimental::test_utils::Inline::RT_scope_controler::key_type key = std::string(__FILE__ " line ") + std::to_string(__LINE__);													\
											gcl::experimental::test_utils::Inline::RT_scope_controler::mapped_type hook_active = RT_scope_controler::IsActive() && gcl::experimental::test_utils::Inline::RT_scope_controler::Get<>(key);	\
											if (hook_active)																																														\
											{																																																		\
												expr																																																\
											}																																																		\
										}

#define CONDITIONAL_SCOPE_INLINE_CALL_ONCE(expr)		{																																																				\
														static const gcl::experimental::test_utils::Inline::RT_scope_controler::key_type key = std::string(__FILE__ " line ") + std::to_string(__LINE__);														\
														gcl::experimental::test_utils::Inline::RT_scope_controler::mapped_type hook_active = RT_scope_controler::IsActive() && gcl::experimental::test_utils::Inline::RT_scope_controler::Get<true>(key);	\
														if (hook_active)																																															\
														{																																																			\
															gcl::functionnal::on_destroy_call onDestroyExecute([](){ gcl::experimental::test_utils::Inline::RT_scope_controler::Desactivate(key); });															\
															expr																																																	\
														}																																																			\
													}

				struct Test
				{
					//struct LogObserver : RT_scope_controler::Observer
					//{
					//	void	NotifyPropertyChanged(RT_scope_controler::value_type & elem) override
					//	{
					//		std::cout << "[+]::[TEST] : New entry at : " << elem.first << std::endl;
					//		elem.second = true;	// activate the element
					//	}
					//};

					// todo : improve that test with value-check
					static bool Proceed(void)
					{
						/*std::shared_ptr<RT_scope_controler::Observer> logObs = std::make_shared<LogObserver>();*/
						// RT_scope_controler::AddObserver(logObs);
						RT_scope_controler::IsActive() = true;

						for (size_t i = 0; i < 5; ++i)
						{
							try
							{
								CONDITIONAL_SCOPE_INLINE_CALL_ONCE(throw std::runtime_error("first throw"););
								CONDITIONAL_SCOPE_INLINE_CALL_ONCE(throw std::runtime_error("second throw"););
								CONDITIONAL_SCOPE_INLINE_CALL_ONCE(throw std::runtime_error("third throw"););
								std::cout << "[+]::[TEST] : " << i << " : No throw" << std::endl;
							}
							catch (const std::exception & ex)
							{
								std::cout << "[+]::[TEST] : " << i << " : exception catch : " << ex.what() << std::endl;
							}
						}
						return true;
					}
				};
			}
		}
	}
}

#endif // GCL_TEST_UTILS__
