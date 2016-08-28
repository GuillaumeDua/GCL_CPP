#ifndef GCL_PATTERN_
# define GCL_PATTERN_

# include <iostream>
# include <queue>
# include <thread>
# include <future>
# include <chrono>
# include <memory>
# include <type_traits>
# include <utility>

namespace GCL
{
	namespace Experimental
	{
		namespace Pattern
		{
			namespace Module // Filter
			{
				template <typename T_ToVisitType>
				struct Type
				{
					virtual void	Visit(T_ToVisitType) = 0;
				};
				template <typename T_ToVisitElement>
				struct Filter
				{
					using _ToVisitElement = T_ToVisitElement;
					virtual void	Visit(_ToVisitElement) = 0;
				};
				template <typename T_ToVisitElement>
				struct FilterChain
				{
					using T_Filter = Filter<T_ToVisitElement>;
					using T_FilterList = std::list<std::shared_ptr<T_Filter>>;
					using _ToVisitElement = typename T_Filter::_ToVisitElement;

					void	Process(T_ToVisitElement & elem)
					{
						for (auto & filter : _filters)
							; // (*filter)(elem); // fixme
					}

					T_FilterList	_filters;
				};
			}
			// Todo : Runnable class with status : Starting, Started, Running, Stoping, Stopped
			template <typename T_ElementType>
			struct ControlCenter final
			{
				RELEASE_INSTRUCTION
				(static_assert(std::is_constructible<T_ElementType>::value == false, "Optimization error : T_ElementType cannot be constructible");)

				using T_ThisType = ControlCenter<T_ElementType>;
				using T_Element			= std::unique_ptr<T_ElementType>;
				using T_FilterChain		= Module::FilterChain<T_Element>;
				using T_ElementQueue	= typename std::queue<T_Element>;
				static_assert(std::is_same<typename T_FilterChain::_ToVisitElement, typename T_ElementQueue::value_type>::value, "Type mismatch");

				ControlCenter() = default;

				static inline		T_ThisType & GetInstance(void)
				{
					static T_ThisType instance;
					return instance;
				}
				static void			Push(T_Element && rEvent)
				{
					T_ThisType & instance = GetInstance();
					instance._Push(std::move(rEvent)); // fixme
				}
				void				_Push(T_Element && rEvent)
				{
					{	// Critical section
						std::unique_lock<std::mutex> lock(_mutex);
						_cache.push(std::move(rEvent));
					}
					_cv.notify_one();
				}
				DEBUG_INSTRUCTION
				(void inline		_EmergencyCacheFlush(void)
				{
					std::unique_lock<std::mutex> lock(_mutex);
					_cache.swap(T_ElementQueue());
				})
				DEBUG_INSTRUCTION
				(static void inline	EmergencyCacheFlush(void)
				{
					T_ThisType & instance = GetInstance();
					instance._EmergencyCacheFlush();
				})

				void				Start(void)
				{
					_running = true;
					if (_thread != nullptr)
						throw std::runtime_error("Event::ControlCenter::Start : Already started");
					_thread.reset();

					struct CaptureAdapter
					{
						std::mutex				& _mutex;
						std::condition_variable & _cv;
						T_ElementQueue			&_cache;
						T_FilterChain			& _filterChain;
					} capture { _mutex, _cv, _cache, _filterChain };

					_thread = std::make_unique<std::thread>([&](CaptureAdapter & capture)
					{
						T_ElementQueue::value_type elem = 0x0;
						while (this->IsRunning())
						{
							if (capture._cache.empty())
							{
								std::unique_lock<std::mutex> lock(capture._mutex);
								capture._cv.wait(lock);
							}
							if (!capture._cache.empty()) // ensure the last elemn is processed even after stop were notified
							{
								{	// Critical section
									std::unique_lock<std::mutex> lock(capture._mutex);
									elem = std::move(capture._cache.front());
									capture._cache.pop();
								}

								capture._filterChain.Process(elem);

								(*elem)();
							}
						}
					}, capture);
				}
				void				Stop(void)
				{
					_running = false;
					if (_thread != nullptr)
					{
						// GetInstance()._cv.notify_all();
						this->_cv.notify_all();
						_thread->join();
					}
					_thread.reset();
				}
				inline bool			IsRunning(void) const
				{
					return _running;
				}
				inline const size_t	Pending(void) const
				{
					return _cache.size();
				}

			protected:
				T_FilterChain	_filterChain;
				T_ElementQueue	_cache;

			private:
				std::unique_ptr<std::thread>	_thread;
				std::mutex						_mutex;
				std::condition_variable			_cv;
				volatile bool					_running = false;

				ControlCenter(const ControlCenter &)	= default;
				ControlCenter(ControlCenter &&)			= delete;
			};

			struct Test
			{
				struct Event final
				{
					Event() = default;
					Event(Event &&) = default;
					void operator()(void)
					{}
				};

				static bool	Proceed(void)
				{
					using ControlCenterType = ControlCenter<Test::Event>;

					{	// Test 0 : nothing to consum
						std::cout << "\t |- Racing ..." << std::endl;
						ControlCenterType::GetInstance().Start();
						std::this_thread::sleep_for(std::chrono::seconds(1));
						ControlCenterType::GetInstance().Stop();
						std::cout << "\t |- Elements remaining: " << ControlCenterType::GetInstance().Pending() << " in 1 second" << std::endl;
						if (ControlCenterType::GetInstance().Pending() != 0)
							return false;
					}

					{	// Test 1 : consuming in 1 second
						static const long long qty = std::chrono::microseconds::period::den;
						std::cout << "\t |- Generating " << qty << " elements ..." << std::endl;
						for (long long i = 0; i < qty; ++i)
							ControlCenterType::Push(std::move(std::make_unique<Event>()));

						std::cout << "\t |- Racing ..." << std::endl;
						ControlCenterType::GetInstance().Start();
						std::this_thread::sleep_for(std::chrono::seconds(1));
						ControlCenterType::GetInstance().Stop();
						std::cout << "\t |- Processed elements : " << (qty - ControlCenterType::GetInstance().Pending()) << " in 1 second" << std::endl; // [+ 200.000, 215.000]
					}

					ControlCenterType::EmergencyCacheFlush();

					{	// Test 2 : Produce take more time than consuming
						std::cout << "\t |- Racing ..." << std::endl;
						ControlCenterType::GetInstance().Start();
						static const long long qty = std::chrono::milliseconds::period::den;
						for (long long i = 0; i < qty; ++i)
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(1));
							ControlCenterType::Push(std::move(std::make_unique<Event>()));
						}
						ControlCenterType::GetInstance().Stop();
						std::cout << "\t |- Processed elements : " << (qty - ControlCenterType::GetInstance().Pending()) << " / " << qty << std::endl;
						if (ControlCenterType::GetInstance().Pending() != 0)
							return false;
					}

					return true;
				}
			};
		}
	}
}

#endif // GCL_PATTERN_