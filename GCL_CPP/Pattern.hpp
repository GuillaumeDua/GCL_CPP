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
					instance._mutex.lock();
					instance._cache.push(rEvent);
					instance._mutex.unlock();
					// notify ?
				}
				static inline T_ThisType & GetInstance(void)
				{
					static T_ThisType instance;
					return instance;
				}

				void Start(void)
				{
					_running = true;

					if (_thread != nullptr)
						throw std::runtime_error("Event::ControlCenter::Start : Already started");

					_thread.reset();

					struct CaptureAdapter
					{
						std::mutex & _mutex;
						T_ElementQueue &_cache;
						T_FilterList & _filters;
					} capture { _mutex, _cache, _filters };

					_thread = std::make_unique<std::thread>([&](CaptureAdapter & capture)
					{
						while (this->IsRunning())
						{
							if (capture._cache.empty())
								; // wait notification, condition_variable, etc ...
							else
							{
								capture._mutex.lock();
								T_ElementQueue::value_type elem = std::move(capture._cache.front());
								capture._cache.pop();
								capture._mutex.unlock();

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
						_thread->join();
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

					
					static const long long qty = std::chrono::microseconds::period::den;
					for (long long i = 0; i < qty; ++i)
						ControlCenterType::Push(Event{});

					std::cout << "\t |- Racing ..." << std::endl;
					ControlCenterType::GetInstance().Start();
					std::this_thread::sleep_for(std::chrono::seconds(1));
					ControlCenterType::GetInstance().Stop();
					std::cout << "\t |- Processed elements : " << (qty - ControlCenterType::GetInstance().Pending()) << " in 1 second" << std::endl;
					return true;
				}
			};
		}
	}
}

#endif // GCL_PATTERN_