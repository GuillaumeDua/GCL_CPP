#ifndef GCL_PATTERN_
# define GCL_PATTERN_

# include <iostream>
# include <queue>
# include <thread>
# include <future>
# include <chrono>

namespace GCL
{
	namespace Experimental
	{
		namespace Pattern
		{
			namespace Module
			{
				template <typename T_ToVisitType>
				struct Type
				{
					virtual void	Visit(T_ToVisitType &) = 0;
				};
				template <typename T_ToVisitType>
				struct ChainElementFilter
				{
					using T_ToVisitElement = typename T_ToVisitType::T_Element;

					virtual void	Visit(T_ToVisitElement &) = 0;
				};
			}
			// Todo : Runnable class with status : Starting, Started, Running, Stoping, Stopped
			template <typename T_ElementType>
			struct ControlCenter final
			{
				using T_Element = T_ElementType;
				using T_ThisType = ControlCenter<T_Element>;

				using T_FilterType = Module::ChainElementFilter<T_ThisType>;
				using T_FilterList = std::list<std::shared_ptr<T_FilterType>>;
				using T_ElementQueue = std::queue<T_Element>;

				static void	Push(T_Element && rEvent)
				{
					T_ThisType & instance = GetInstance();
					{	// Critical section
						std::unique_lock<std::mutex> lock(instance._mutex);
						instance._cache.push(rEvent);
					}
					instance._cv.notify_one();
				}
				static inline T_ThisType & GetInstance(void)
				{
					static T_ThisType instance;
					return instance;
				}

				DEBUG_INSTRUCTION
				(static void inline EmergencyCacheFlush(void)
				{
					ControlCenter & instance = GetInstance();
					std::unique_lock<std::mutex> lock(instance._mutex);
					instance._cache.swap(T_ElementQueue());
				})

				void Start(void)
				{
					_running = true;

					if (_thread != nullptr)
						throw std::runtime_error("Event::ControlCenter::Start : Already started");

					_thread.reset();

					struct CaptureAdapter
					{
						std::mutex & _mutex;
						std::condition_variable & _cv;
						T_ElementQueue &_cache;
						T_FilterList & _filters;
					} capture { _mutex, _cv, _cache, _filters };

					_thread = std::make_unique<std::thread>([&](CaptureAdapter & capture)
					{
						T_ElementQueue::value_type elem;
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

								for (auto & filter : capture._filters)
									filter->Visit(elem);
								elem();
							}
						}
					}, capture);
				}
				void Stop(void)
				{
					_running = false;
					if (_thread != nullptr)
					{
						GetInstance()._cv.notify_all();
						_thread->join();
					}
					_thread.reset();
				}
				inline bool	IsRunning(void) const
				{
					return _running;
				}
				inline const size_t Pending(void) const
				{
					return _cache.size();
				}

			protected:
				T_FilterList	_filters;
				T_ElementQueue	_cache;

				void ProcessElement(T_Element & elem)
				{
					for (auto & filter : _filters)
						filter->Visit(elem);
					elem();
				}

			private:
				std::unique_ptr<std::thread>	_thread;
				std::mutex						_mutex;
				std::condition_variable			_cv;
				volatile bool					_running = false;

				ControlCenter() = default;
				ControlCenter(const ControlCenter &) = default;
				ControlCenter(ControlCenter &&) = delete;
			};

			struct Test
			{
				struct Event final
				{
					explicit Event() = default;
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
							ControlCenterType::Push(Event{});

						std::cout << "\t |- Racing ..." << std::endl;
						ControlCenterType::GetInstance().Start();
						std::this_thread::sleep_for(std::chrono::seconds(1));
						ControlCenterType::GetInstance().Stop();
						std::cout << "\t |- Processed elements : " << (qty - ControlCenterType::GetInstance().Pending()) << " in 1 second" << std::endl;
					}

					ControlCenterType::EmergencyCacheFlush();

					{	// Test 2 : Produce take more time than consuming
						std::cout << "\t |- Racing ..." << std::endl;
						ControlCenterType::GetInstance().Start();
						static const long long qty = std::chrono::milliseconds::period::den;
						for (long long i = 0; i < qty; ++i)
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(1));
							ControlCenterType::Push(Event{});
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