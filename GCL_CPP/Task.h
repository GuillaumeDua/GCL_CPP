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

# include "Thread.h"

namespace GCL
{
	namespace Task
	{
		template <typename ElementType>
		struct	Queue
		{
			explicit Queue() = default;
			Queue(const Queue &) = delete;
			Queue(const Queue &&) = delete;
			Queue & operator=(const Queue &) = delete;
			~Queue() = default;

			using _ElementType = ElementType;

			void						Push(ElementType * elem)
			{
				std::lock_guard<std::mutex>	guardLock(_mutex);
				_content.push(elem);
				_conditionVariable.notify_one();	// [Todo] : Test
				//_conditionVariable.notify_all();
			}
			ElementType *				Pop(void)
			{
				ElementType * elem;
				std::lock_guard<std::mutex>	guardLock(_mutex);
				if (this->_content.size() == 0)
					return 0x0;
				elem = this->_content.front();
				this->_content.pop();
				return elem;
			}
			const size_t				Size(void)
			{
				std::lock_guard<std::mutex>	guardLock(_mutex);
				return this->_content.size();
			}
			void						WaitFor(void)
			{
				std::unique_lock<std::mutex>	uniqueLock(_mutex);
				_conditionVariable.wait(uniqueLock);
			}
			void						StopAllPendingWait()
			{
				_conditionVariable.notify_all();
			}

		protected:
			std::mutex					_mutex;
			std::condition_variable		_conditionVariable;
			std::queue<ElementType*>	_content;
		};

		template <typename QueueType>
		struct	Producer
		{
			Producer(QueueType & queue)
				: _queue(queue)
			{}


		protected:
			QueueType	&	_queue;
		};

		template <typename QueueType, class HandleElementPolicy>
		struct	Consumer
		{
			static_assert(std::is_same
				<
				typename QueueType::_ElementType,
				typename HandleElementPolicy::_ElementType
				>::value,
				"[Error]::[GCL::Consumer] : Mismatch elements type"
				);

			explicit Consumer(QueueType & queue)
				: _queue(queue)
				, _isRunning(false)
				, _stopRequieredIfLazy(false)
			{}
			Consumer() = delete;
			Consumer(const Consumer &) = delete;
			Consumer(const Consumer &&) = delete;
			Consumer & operator=(const Consumer &) = delete;
			~Consumer() = default;

			const bool	IsRunning(void) const
			{
				return this->_isRunning;
			}
			const bool	IsStopRequieredIfLazy(void) const
			{
				return this->_stopRequieredIfLazy;
			}
			void		Work(void)
			{
				GCL::Thread::scout.Print("[+] : Consumer : Work starting");
				QueueType::_ElementType * element;
				_isRunning = true;
				_stopRequieredIfLazy = false;

				while (_isRunning)
				{
					if ((element = this->_queue.Pop()) != 0x0)
						HandleElementPolicy::HandleElement(element);
					else if (_stopRequieredIfLazy)
						break;
					else
						this->_queue.WaitFor();
				}
				_isRunning = false;
			}
			inline void	Stop(void)
			{
				this->_isRunning = false;
			}
			inline void	RequiereStopIfLazy(void)
			{
				this->_stopRequieredIfLazy = true;
			}

		protected:
			volatile bool	_isRunning;
			volatile bool	_stopRequieredIfLazy;
			QueueType	&	_queue;
		};


		struct Test
		{
			template <typename ElementType>
			struct	FakeHandleElementPolicy
			{
				using _ElementType = ElementType;

				static bool	HandleElement(ElementType * element)
				{
					bool returnValue(element != 0x0);
					THREAD_SAFE_STDCOUT("[-] : Consumer : " << std::setw(4) << std::this_thread::get_id() << " : [0x" << element << "] => [" << *element << "]");
					delete element;
					std::this_thread::sleep_for(std::chrono::milliseconds(300));
					return returnValue;
				}
			};

			template <typename QueueType, typename ConsumerType, size_t Qty>
			struct	ConsumerPool
			{
				explicit ConsumerPool(QueueType & queue)
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

			template <typename ElementType>
			struct FakeUniqueRscCreationPolicy
			{
				using _ElementType = ElementType;

				static ElementType * GetNewInstance(void)
				{
					return new ElementType();
				}
			};

			bool Proceed(void)
			{
				using ElementType = int;
				using QueueType = GCL::Task::Queue < ElementType > ;
				using HandleElementPolicy = FakeHandleElementPolicy < ElementType > ;
				using ConsumerType = GCL::Task::Consumer < QueueType, HandleElementPolicy > ;

				// [Producer]
				/*QueueType	queue;
				auto		producingAsync = std::async(std::launch::async, [&queue](QueueType & queue) mutable
				{
				ElementType * element;
				for (size_t i = 0; i < 100; ++i)
				{
				element = new ElementType(i);
				THREAD_SAFE_STDCOUT("[+] : Producer : " << std::setw(4) << std::this_thread::get_id() << " : [0x" << element << "] => [" << *element << "]");
				queue.Push(element);
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
				}, std::ref(queue));
				*/
				// [1 consumer]
				//GCL::Consumer<QueueType, HandleElementPolicy> consumer(queue);
				//auto		consumerAsync = std::async(std::launch::async, [&consumer](GCL::Consumer<QueueType, HandleElementPolicy> & consumer) mutable
				//{
				//	consumer.Work();
				//}, std::ref(consumer));
				//producingAsync.get();
				//consumer.RequiereStopIfLazy();
				//consumerAsync.get();

				// [n(2) consumers]
				//ConsumerPool<QueueType, ConsumerType, 2> consumers(queue);
				//consumers.LauchASync();
				//producingAsync.get();
				//consumers.Apply(std::function<void(ConsumerType &)>([&](ConsumerType & consumer) mutable -> void
				//{
				//	consumer.RequiereStopIfLazy();
				//}));
				//queue.StopAllPendingWait();
				//consumers.GetSync();

				// [Unique Rcs] (very trash test)
				GCL::Thread::UniqueRsc<ElementType, FakeUniqueRscCreationPolicy<ElementType> >	uniqueRsc;
				std::vector<std::future<void>>	_vec;
				for (size_t i = 0; i < 10; ++i)
					_vec.emplace_back(std::async(std::launch::async, [&uniqueRsc](void) -> void
				{
					for (size_t i = 0; i < 5; ++i)
					{
						THREAD_SAFE_STDCOUT('[' << std::setw(8) << std::this_thread::get_id() << "] -> rcs=[0x" << &(uniqueRsc.Get()) << ']');
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