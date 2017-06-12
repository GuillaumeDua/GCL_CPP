#ifndef __GCL_PRODUCER_CONSUMER__
# define __GCL_PRODUCER_CONSUMER__

# include <iostream>
# include <functional>
# include <future>
# include <condition_variable>
# include <mutex>
# include <type_traits>
# include <queue>
# include <iomanip>

# include <gcl_cpp/thread.hpp>
# include <gcl_cpp/preprocessor.hpp>

namespace gcl
{
	namespace task
	{
		template <typename T_element_type>
		struct	queue
		{
			explicit queue() = default;
			queue(const queue &) = delete;
			queue(const queue &&) = delete;
			queue & operator=(const queue &) = delete;
			~queue() = default;

			using element_type = T_element_type;

			void						push(T_element_type * elem)
			{
				std::lock_guard<std::mutex>	guardLock(_mutex);
				_content.push(elem);
				_conditionVariable.notify_one();	// [Todo] : Test
				//_conditionVariable.notify_all();
			}
			T_element_type *			pop(void)
			{
				T_element_type * elem;
				std::lock_guard<std::mutex>	guardLock(_mutex);
				if (this->_content.size() == 0)
					return 0x0;
				elem = this->_content.front();
				this->_content.pop();
				return elem;
			}
			const size_t				size(void)
			{
				std::lock_guard<std::mutex>	guardLock(_mutex);
				return this->_content.size();
			}
			void						wait_for(void)
			{
				std::unique_lock<std::mutex>	uniqueLock(_mutex);
				_conditionVariable.wait(uniqueLock);
			}
			void						stop_pending_waits()
			{
				_conditionVariable.notify_all();
			}

		protected:
			std::mutex					_mutex;
			std::condition_variable		_conditionVariable;
			std::queue<T_element_type*>	_content;
		};

		template <typename T_queue>
		struct	producer
		{
			using queue_type = T_queue;

			producer(queue_type & queue)
				: _queue(queue)
			{}

		protected:
			queue_type	&	_queue;
		};

		template <typename T_queue, class T_HandleElementPolicy>
		struct	consumer
		{
			using queue_type = T_queue;

			static_assert
			(
				std::is_same
				<
					typename T_queue::element_type,
					typename T_HandleElementPolicy::element_type
				>::value,
				"[Error]::[gcl::consumer] : Mismatch elements type"
			);

			explicit consumer(T_queue & queue)
				: _queue(queue)
				, is_running(false)
				, stop_requiered_if_lazy(false)
			{}
			consumer() = delete;
			consumer(const consumer &) = delete;
			consumer(const consumer &&) = delete;
			consumer & operator=(const consumer &) = delete;
			~consumer() = default;

			const bool	IsRunning(void) const
			{
				return this->is_running;
			}
			const bool	IsStopRequieredIfLazy(void) const
			{
				return this->stop_requiered_if_lazy;
			}
			void		Work(void)
			{
				gcl::thread::scout.Print("[+] : consumer : Work starting");
				queue_type::element_type * element;
				is_running = true;
				stop_requiered_if_lazy = false;

				while (is_running)
				{
					if ((element = this->_queue.pop()) != 0x0)
						T_HandleElementPolicy::HandleElement(element);
					else if (stop_requiered_if_lazy)
						break;
					else
						this->_queue.wait_for();
				}
				is_running = false;
			}
			inline void	Stop(void)
			{
				this->is_running = false;
			}
			inline void	RequiereStopIfLazy(void)
			{
				this->stop_requiered_if_lazy = true;
			}

		protected:
			volatile bool	is_running;
			volatile bool	stop_requiered_if_lazy;
			queue_type	&	_queue;
		};


		struct Test // todo : use gcl::test
		{
			template <typename T_element_type>
			struct	FakeHandleElementPolicy
			{
				using element_type = T_element_type;

				static bool	HandleElement(T_element_type * element)
				{
					bool returnValue(element != 0x0);
					THREAD_SAFE_STDCOUT("[-] : consumer : " << std::setw(4) << std::this_thread::get_id() << " : [0x" << element << "] => [" << *element << "]");
					delete element;
					std::this_thread::sleep_for(std::chrono::milliseconds(300));
					return returnValue;
				}
			};

			template <typename T_queue, typename ConsumerType, size_t Qty>
			struct	ConsumerPool
			{
				explicit ConsumerPool(T_queue & queue)
				{
					for (size_t i = 0; i < Qty; ++i)
						this->_content.push_back(new ConsumerType(queue));
				}
				ConsumerPool(const ConsumerPool &) = delete;
				ConsumerPool(const ConsumerPool &&) = delete;
				ConsumerPool & operator=(const ConsumerPool &) = delete;
				~ConsumerPool()
				{
					for (auto & consumerPtr : _content)
						delete consumerPtr;
				}

				void	Apply(std::function<void(ConsumerType&)> & func)
				{
					for (auto & consumerPtr : _content)
						func(*consumerPtr);
				}
				void	LauchASync(void)
				{
					for (auto & consumerPtr : _content)
					{
						this->_asyncFutures.push_back(
						{
							std::async(std::launch::async, [&](ConsumerType * consumer) mutable	// Osef, je copie le pointer, c'est cheap
							{
								consumer->Work();
							}, consumerPtr)
						});
					}
				}
				void	GetSync(void)
				{
					for (auto & future : _asyncFutures)
						future.get();
				}

			protected:
				std::vector<ConsumerType*>		_content;
				std::vector<std::future<void> >	_asyncFutures;
			};

			template <typename T_element_type>
			struct FakeUniqueRscCreationPolicy
			{
				using element_type = T_element_type;

				static T_element_type * GetNewInstance(void)
				{
					return new T_element_type();
				}
			};

			static bool Proceed(void)
			{
				using T_element_type = int;
				using T_queue = gcl::task::queue < T_element_type > ;
				using T_HandleElementPolicy = FakeHandleElementPolicy < T_element_type > ;
				using ConsumerType = gcl::task::consumer < T_queue, T_HandleElementPolicy > ;

				// [producer]
				/*T_queue	queue;
				auto		producingAsync = std::async(std::launch::async, [&queue](T_queue & queue) mutable
				{
				T_element_type * element;
				for (size_t i = 0; i < 100; ++i)
				{
				element = new T_element_type(i);
				THREAD_SAFE_STDCOUT("[+] : producer : " << std::setw(4) << std::this_thread::get_id() << " : [0x" << element << "] => [" << *element << "]");
				queue.push(element);
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
				}, std::ref(queue));
				*/
				// [1 consumer]
				//gcl::consumer<T_queue, T_HandleElementPolicy> consumer(queue);
				//auto		consumerAsync = std::async(std::launch::async, [&consumer](gcl::consumer<T_queue, T_HandleElementPolicy> & consumer) mutable
				//{
				//	consumer.Work();
				//}, std::ref(consumer));
				//producingAsync.get();
				//consumer.RequiereStopIfLazy();
				//consumerAsync.get();

				// [n(2) consumers]
				//ConsumerPool<T_queue, ConsumerType, 2> consumers(queue);
				//consumers.LauchASync();
				//producingAsync.get();
				//consumers.Apply(std::function<void(ConsumerType &)>([&](ConsumerType & consumer) mutable -> void
				//{
				//	consumer.RequiereStopIfLazy();
				//}));
				//queue.stop_pending_waits();
				//consumers.GetSync();

				// [Unique Rcs] (very trash test)
				gcl::thread::unique_rsc<T_element_type, FakeUniqueRscCreationPolicy<T_element_type> >	uniqueRsc;
				std::vector<std::future<void>>	_vec;
				for (size_t i = 0; i < 10; ++i)
					_vec.emplace_back(std::async(std::launch::async, [&uniqueRsc](void) -> void
				{
					for (size_t i = 0; i < 5; ++i)
					{
						const int * unique_rcs_ptr = &(uniqueRsc.Get());
						GCL_DEBUG_INSTRUCTION(THREAD_SAFE_STDCOUT('[' << std::setw(8) << std::this_thread::get_id() << "] -> rcs=[0x" << unique_rcs_ptr << ']'));
						std::this_thread::sleep_for(std::chrono::seconds(1));
					}
				}));
				for (auto & elem : _vec)
					elem.get();
				return true;
			}
		};
	}
}





#endif // __GCL_PRODUCER_CONSUMER__