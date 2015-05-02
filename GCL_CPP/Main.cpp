# include "ProdConsum.h"
# include <chrono>
# include <thread>
# include <iomanip>

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

int	main(int ac, char* av[])
{
	using ElementType = int;
	using QueueType = GCL::Queue < ElementType > ;
	using HandleElementPolicy = FakeHandleElementPolicy < ElementType > ;
	using ConsumerType = GCL::Consumer < QueueType, HandleElementPolicy > ;

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

	system("pause");
	return 0;
}