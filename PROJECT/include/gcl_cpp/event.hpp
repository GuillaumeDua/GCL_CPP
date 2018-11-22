#ifndef GGE_EVENTS__
# define GGE_EVENTS__

#include <iostream>
#include <array>
#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <future>

#include <gcl_cpp/type_info.hpp>
#include <gcl_cpp/preprocessor.hpp>

namespace gcl
{
	struct event_default_type
	{
		virtual inline ~event_default_type() = 0 {};
	};

	template <class event_interface_t = event_default_type>
	struct event final
	{
		GCL_PREPROCESSOR__NOT_INSTANTIABLE(event);

		using interface_t = event_interface_t;

		struct handler
		{
			using typeinfo_holder_t = gcl::deprecated::type_info::holder<interface_t>;
			using event_callback_t = std::function<void(const interface_t &)>;

			handler() = default;
			handler(handler &&) = delete;
			handler(handler &) = delete;
			virtual ~handler() {}

			template <typename event_t>
			void on(const event_t & ev)
			{
				static_assert(std::is_base_of<interface_t, event_t>::value, "event_t does not derive from interface_t");
				this->on(gcl::deprecated::type_info::id<event_t>::value, ev);
			}
			void on(const typeinfo_holder_t & holder)
			{
				this->on(holder.id, *(holder.value));
			}
			virtual void on(gcl::deprecated::type_info::id_type event_id, const interface_t & ev) = 0;

		//protected:
			template <typename event_t>
			void add_listener(event_callback_t && callback)
			{
				add_listener(gcl::deprecated::type_info::id<event_t>::value, std::forward<event_callback_t>(callback));
			}
			virtual void add_listener(gcl::deprecated::type_info::id_type event_id, event_callback_t && callback) = 0;
		};

		struct one_to_one_handler : handler
		{
			using event_callback_t = handler::event_callback_t;
			using socket_container_t = std::pair<gcl::type_info::id::type, event_callback_t>;

			one_to_one_handler() = default;
			one_to_one_handler(gcl::type_info::id::type event_id, event_callback_t && cb)
				: socket(event_id, std::forward<event_callback_t>(cb))
			{}
			one_to_one_handler(socket_container_t && socket)
				: socket(socket)
			{}

			void on(gcl::type_info::id::type event_id, const interface_t & ev) override
			{
				if (event_id != socket.first)
					throw std::domain_error("one_to_one_handler::on : bad event_id");
				socket.second(ev);
			}
			void add_listener(gcl::type_info::id::type event_id, event_callback_t && callback) override
			{
				socket.first = event_id;
				socket.second = std::move(callback);
			}

		protected:
			socket_container_t socket;
		};
		struct one_to_many_handler : handler
		{
			using event_callback_t = handler::event_callback_t;
			using socket_container_t = std::pair<gcl::type_info::id::type, std::vector<event_callback_t>>;

			one_to_many_handler() = default;
			one_to_many_handler(gcl::type_info::id::type event_id, event_callback_t && cb)
				: socket(event_id, { std::forward<event_callback_t>(cb) })
			{}
			one_to_many_handler(gcl::type_info::id::type event_id, typename socket_container_t::second_type && cbs)
				: socket(event_id, std::forward<socket_container_t::second_type>(cbs) )
			{}
			one_to_many_handler(socket_container_t && socket)
				: socket(socket)
			{}

			void on(gcl::type_info::id::type event_id, const interface_t & ev) override
			{
				if (event_id != socket.first)
					throw std::domain_error("one_to_one_handler::on : bad event_id");
				for (auto & cb : socket.second)
					cb(ev);
			}
			void add_listener(gcl::type_info::id::type event_id, event_callback_t && callback) override
			{
				if (socket.first != event_id)
				{
					socket.second.clear();
					socket.first = event_id;
				}
				socket.second.push_back(std::move(callback));
			}

		protected:
			socket_container_t socket;
		};
		struct many_to_one_handler : handler
		{
			using event_callback_t = handler::event_callback_t;
			using socket_container_t = std::unordered_map<gcl::deprecated::type_info::id_type, event_callback_t>;

			many_to_one_handler() = default;
			many_to_one_handler(std::initializer_list<typename socket_container_t::value_type> && il)
				: sockets{ il }
			{}

			void on(gcl::deprecated::type_info::id_type event_id, const interface_t & ev) override
			{
				sockets.at(event_id)(ev);
			}
			void add_listener(gcl::deprecated::type_info::id_type event_id, event_callback_t && callback)  override
			{
				sockets[event_id] = callback;
			}

		protected:
			socket_container_t sockets;
		};
		struct many_to_many_handler : handler
		{
			using event_callback_t = handler::event_callback_t;
			using socket_container_t = std::unordered_multimap<gcl::deprecated::type_info::id_type, event_callback_t>;

			many_to_many_handler() = default;
			many_to_many_handler(std::initializer_list<typename socket_container_t::value_type> && il)
				: sockets{ il }
			{}

			void on(gcl::deprecated::type_info::id_type event_id, const interface_t & ev) override
			{
				auto range = sockets.equal_range(event_id);
				for (auto it = range.first; it != range.second; ++it)
					it->second(ev);
			}
			void add_listener(gcl::deprecated::type_info::id_type event_id, event_callback_t && callback)  override
			{
				sockets.insert(socket_container_t::value_type{ event_id, callback });
			}

		protected:
			socket_container_t sockets;
		};

		struct select_handler final
		{
			GCL_PREPROCESSOR__NOT_INSTANTIABLE(select_handler);

			enum relation_val_t
			{
				one,
				many
			};

			template <relation_val_t first_val, relation_val_t second_val>
			struct get;

			template <>
			struct get<one, one>
			{
				using handler_t = one_to_one_handler;
			};
			template <>
			struct get<one, many>
			{
				using handler_t = one_to_many_handler;
			};
			template <>
			struct get<many, one>
			{
				using handler_t = many_to_one_handler;
			};
			template <>
			struct get<many, many>
			{
				using handler_t = many_to_many_handler;
			};
		};

		template <typename T>
		struct route
		{
			using value_type = T;
			using node_type = std::function<void(const value_type &)>;
			using node_list_type = typename std::vector<node_type>;

			route() = default;
			route(std::initializer_list<typename node_list_type::value_type> il)
				: nodes{ il }
			{}
			route(const route &) = default;
			route(const route &&) = delete;

			void add_node(node_type && node)
			{
				nodes.emplace_back(std::move(node));
			}
			void drive_sync(const value_type & elem)
			{
				for (auto & node : nodes)
				{
					node(elem);
				}
			}
			template <class InputIterator>
			void drive_sync(InputIterator first, InputIterator last)
			{
				while (first != last)
				{
					drive_sync(*first);
					++first;
				}
			}
			template <class InputIterator>
			void drive_async(InputIterator first, InputIterator last)
			{
				using future_type = std::future<void>;
				std::vector<future_type>	futures;
				std::vector<std::mutex>		node_mutexes(nodes.size(), std::mutex{});

				while (first != last)
				{
					futures.emplace_back(std::move(std::async(std::launch::async, [&mutexes]() {
						for (uint32_t i = 0; i < nodes.size(); ++i)
						{
							std::unique_lock<std::mutex> lock(mutexes[i]);
							nodes[i](elem);
						}
					})));
					++first;
				}

				for (auto & future : futures)
					future.get();
			}

		protected:
			node_list_type	nodes;
		};

		struct dispatcher
		{	// subject
			using handler_t = handler;
			using holder_event_t = gcl::deprecated::type_info::holder<interface_t>;

			template <typename event_t>
			void subscribe(const std::shared_ptr<handler_t> & handler)
			{
				subscribers[gcl::deprecated::type_info::id<event_t>::value].insert(handler);
			}
			template <typename event_t>
			void unsubscribe(const std::shared_ptr<handler_t> & handler)
			{
				subscribers.at(gcl::type_info::id<event_t>::value).erase(handler);
			}

			template <typename event_t>
			void dispatch(const event_t & ev)
			{
				static_assert(std::is_base_of<event::interface_t, event_t>::value, "event_t does not derive from event::interface_t");

				holder_event_t holder{ std::move(new event_t{ev}) };
				for (auto & subscriber : subscribers.at(gcl::deprecated::type_info::id<event_t>::value))
					subscriber->on(holder);
			}
			void dispatch(const holder_event_t & holder)
			{
				for (auto & subscriber : subscribers.at(holder.id))
					subscriber->on(holder);
			}
			void dispatch(gcl::type_info::id::type event_id, const interface_t & ev)
			{
				for (auto & subscriber : subscribers.at(event_id))
					subscriber->on(event_id, ev);
			}

		protected:
			using handler_set_t = std::unordered_set<std::shared_ptr<handler_t>>;
			std::unordered_map<gcl::deprecated::type_info::id_type, handler_set_t> subscribers;
		};

		struct system
		{
			using event_holder_t = gcl::deprecated::type_info::holder<interface_t>;

			template <typename event_t>
			void process(const event_t & ev)
			{
				static_assert(std::is_base_of<event::interface_t, event_t>::value, "event_t do not derive from interface_t");
				process(gcl::type_info::id<T>::value, ev);
			}
			void process(gcl::type_info::id::type event_id, const interface_t & ev)
			{
				using event_ptr_t = event::interface_t*;
				using event_unique_ptr_t = std::unique_ptr<event::interface_t>;

				event_ptr_t tmp = const_cast<event_ptr_t>(&ev);
				process(event_holder_t(event_id, event_unique_ptr_t(std::move(tmp))));
			}
			void process(const gcl::deprecated::type_info::holder<interface_t> & holder)
			{
				route.drive_sync(holder);
				dispatcher.dispatch(holder);
			}

			using event_route_t = route<event_holder_t>;
			event_route_t route;
			dispatcher dispatcher;
		};

		struct experimental final
		{
			template <typename ... t_events>
			struct static_socket
			{
				using socket_callback = std::function<void()>;
				using socket_type = std::vector<socket_callback>;
				using tuple_info_t = gcl::type_info::variadic_template<t_events...>;

				template <typename T>
				socket_type & on()
				{
					return sockets[tuple_info_t::index_of<T>()];
				}
				template <typename T>
				void add(socket_callback cb)
				{
					sockets[tuple_info_t::index_of<T>()].push_back(cb);
				}

				template <typename T>
				void trigger()
				{
					for (auto & elem : on<T>())
						elem();
				}
				template <size_t N>
				void trigger()
				{
					for (auto & elem : sockets[N])
						elem();
				}
				template <typename T>
				void trigger(const T &)
				{
					trigger<T>();
				}
				template <>
				void trigger<size_t>(const size_t & N)
				{
					trigger<N>();
				}

			protected:

				std::array<socket_type, sizeof...(t_events)> sockets;
			};
		};
	};
}

#endif // GGE_EVENTS__