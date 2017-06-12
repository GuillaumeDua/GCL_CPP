#ifndef __GCL_THREAD__
# define __GCL_THREAD__

# include <iostream>
# include <functional>
# include <future>
# include <mutex>
# include <type_traits>
# include <unordered_map>
# include <sstream>

namespace gcl
{
	namespace thread
	{
		// Warning : Create a bottle-neck on ressources acquisition
		// todo : allocator instead/plus T_element_creation_policy ?
		template <typename T_element_type, typename T_element_creation_policy>
		struct	unique_rsc
		{
			using element_type = std::remove_pointer_t<T_element_type>; // std::decay
			using element_creator_fun_t = std::function<element_type*(void)>;

			static_assert
			(
				std::is_same
				<
					typename element_type,
					typename T_element_creation_policy::element_type
				>::value,
				"[Error]::[gcl::thread::unique_rsc] : Mismatch elements type"
			);

			template <typename Elem> element_type &				Emplace(std::initializer_list<Elem> & initializer_list)
			{
				const std::thread::id id = std::this_thread::get_id();
				std::lock_guard<std::mutex>	lock(_mutex);

				if (this->_content.find(id) != this->_content.end())
					throw std::runtime_error("[Error] : Ressource already exists");
				this->_content[id] = T_element_creation_policy::GetNewInstance(initializer_list);
				return *(this->_content[id]);
			}
			element_type &										Emplace(element_creator_fun_t & createFunc)
			{
				std::lock_guard<std::mutex>	lock(_mutex);

				const std::thread::id id = std::this_thread::get_id();
				if (this->_content.find(id) != this->_content.end())
					throw std::runtime_error("[Error] : Ressource already exists");
				this->_content[id] = createFunc();
				return *(this->_content[id]);
			}
			element_type &										Get(void)
			{
				const std::thread::id id = std::this_thread::get_id();
				std::lock_guard<std::mutex>	lock(_mutex);

				if (this->_content.find(id) == this->_content.end())
					this->_content[id] = T_element_creation_policy::GetNewInstance();
				return *(this->_content[id]);
			}
			const element_type &									Get(void) const
			{
				std::lock_guard<std::mutex>	lock(_mutex);
				if (this->_content.find(std::this_thread::get_id()) == this->_content.end())
					throw std::runtime_error("[Error] : gcl::thread::unique_rsc : Rsc does not exists, and cannot create it (const qualifier)");
				return this->_content[std::this_thread::get_id()];
			}
			void												Clean(void)
			{
				std::lock_guard<std::mutex>	lock(_mutex);
				for (auto & elemPtr : _content)
					delete elemPtr;
			}

		protected:
			std::unordered_map<std::thread::id, element_type*>	_content;
			std::mutex												_mutex;
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
	gcl::thread::scout.Print(__ossTmpMsg.str());	\
							}
	}
}

#endif // __GCL_THREAD__