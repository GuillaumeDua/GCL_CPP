#ifndef __GCL_PRODUCER_CONSUMER__
# define __GCL_PRODUCER_CONSUMER__

# include <iostream>
# include <functional>
# include <future>
# include <condition_variable>
# include <mutex>
# include <type_traits>
# include <queue>
# include <unordered_map>
# include <sstream>

namespace GCL
{
	namespace Thread
	{
		template <typename ElementType, typename ElementCreationPolicy>
		struct	UniqueRsc
		{
			using _ElementType = ElementType;
			using _ElementCreationFunc = std::function < ElementType*(void) >;

			static_assert(std::is_same
				<
				typename _ElementType,
				typename ElementCreationPolicy::_ElementType
				>::value,
				"[Error]::[GCL::Thread::UniqueRsc] : Mismatch elements type"
				);

			template <typename Elem> ElementType &				Emplace(std::initializer_list<Elem> & initializer_list)
			{
				const std::thread::id id = std::this_thread::get_id();
				std::lock_guard<std::mutex>	lock(_mutex);

				if (this->_content.find(id) != this->_content.end())
					throw std::runtime_error("[Error] : Ressource already exists");
				this->_content[id] = ElementCreationPolicy::GetNewInstance(initializer_list);
				return *(this->_content[id]);
			}
			ElementType &										Emplace(_ElementCreationFunc & createFunc)
			{
				std::lock_guard<std::mutex>	lock(_mutex);

				const std::thread::id id = std::this_thread::get_id();
				if (this->_content.find(id) != this->_content.end())
					throw std::runtime_error("[Error] : Ressource already exists");
				this->_content[id] = createFunc();
				return *(this->_content[id]);
			}
			ElementType &										Get(void)
			{
				const std::thread::id id = std::this_thread::get_id();
				std::lock_guard<std::mutex>	lock(_mutex);
				
				if (this->_content.find(id) == this->_content.end())
					this->_content[id] = ElementCreationPolicy::GetNewInstance();
				return *(this->_content[id]);
			}
			const ElementType &									Get(void) const
			{
				std::lock_guard<std::mutex>	lock(_mutex);
				if (this->_content.find(std::this_thread::get_id()) == this->_content.end())
					throw std::runtime_error("[Error] : GCL::Thread::UniqueRsc : Rsc does not exists, and cannot create it (const qualifier)");
				return this->_content[std::this_thread::get_id()];
			}
			void												Clean(void)
			{
				std::lock_guard<std::mutex>	lock(_mutex);
				for (auto & elemPtr : _content)
					delete elemPtr;
			}

		protected:
			std::unordered_map<std::thread::id, ElementType*>	_content;
			std::mutex											_mutex;
		};

		struct	SafeStdCout
		{
			SafeStdCout() = default;
			SafeStdCout(const SafeStdCout &) = delete;
			SafeStdCout(const SafeStdCout &&) = delete;
			SafeStdCout & operator=(const SafeStdCout &) = delete;
			~SafeStdCout() = default;

			template<typename T, typename... Args>
			void Print(T value, Args... args)
			{
				{
					std::lock_guard<std::mutex>	lockGuard(this->_mutex);
					std::cout << value;
					std::cout.flush();
				}
				Print(args...);
			}
			template<typename T>
			void Print(T value)
			{
				std::lock_guard<std::mutex>	lockGuard(this->_mutex);
				std::cout << value << std::endl;
			}

			template <typename T>
			SafeStdCout &	operator<<(const T & data)
			{
				{
					std::lock_guard<std::mutex>	lockGuard(this->_mutex);
					std::cout << data;
					std::cout.flush();
				}
				return *this;
			}
		protected:
			std::mutex	_mutex;
		}		static scout;

		//[Quick-fix]
# define THREAD_SAFE_STDCOUT(args) {				\
	std::ostringstream __ossTmpMsg;					\
	__ossTmpMsg << args;							\
	GCL::Thread::scout.Print(__ossTmpMsg.str());	\
				}
	}

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