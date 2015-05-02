#ifndef __GCL_PRODUCER_CONSUMER__
# define __GCL_PRODUCER_CONSUMER__

# include <iostream>
# include <functional>
# include <future>
# include <condition_variable>
# include <mutex>
# include <type_traits>
# include <queue>

# include "Thread.h"

namespace GCL
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
}





#endif // __GCL_PRODUCER_CONSUMER__