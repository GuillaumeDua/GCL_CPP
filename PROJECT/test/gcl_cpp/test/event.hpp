#pragma once // not using standard header-guard in order to not pollute macro completion on GCL_TEST*

#include <gcl_cpp/event.hpp>
#include <gcl_cpp/test.hpp>

#include <string>
#include <type_traits>

namespace gcl
{
	namespace test
	{
		struct event
		{
			using gcl_event_t = gcl::event<>;

			struct A_event : gcl_event_t::interface_t
			{
				A_event() = default;
				explicit A_event(int value)
					: a_var(value)
				{}
				int a_var = 0;

				static std::size_t counter;
			};
			struct B_event : gcl_event_t::interface_t
			{
				B_event() = default;
				B_event(std::string && value)
					: b_var(value)
				{}
				std::string b_var = "default";

				static std::size_t counter;
			};
			struct C_event : gcl_event_t::interface_t
			{
				C_event() = default;
				explicit C_event(int value)
					: c_var(value)
				{}
				int c_var = 0;

				static std::size_t counter;
			};

			struct handler
			{
				static void proceed()
				{
					using handler_t = gcl_event_t::select_handler::get
					<
						gcl_event_t::select_handler::many,
						gcl_event_t::select_handler::one
					>::handler_t;
					static_assert(std::is_same<handler_t, gcl_event_t::many_to_one_handler>::value, "gcl::test::event::handler::proceed : select_handler fail");

					std::unique_ptr<gcl_event_t::handler> handler(new handler_t
					{
						{ gcl::deprecated::type_info::id<B_event>::value, [](const gcl_event_t::interface_t &) { B_event::counter++; } },
						{ gcl::deprecated::type_info::id<C_event>::value, [](const gcl_event_t::interface_t &) { C_event::counter++; } }
					});

					handler->add_listener<A_event>([](const gcl_event_t::interface_t &) { A_event::counter++; });

					GCL_TEST__EXPECT(A_event::counter == 0, "gcl::test::event::handler : bad A_event::counter value");
					handler->on(A_event{});
					GCL_TEST__EXPECT(A_event::counter == 1, "gcl::test::event::handler : bad A_event::counter value");

					gcl_event_t::interface_t * ev = new B_event();
					GCL_TEST__EXPECT(B_event::counter == 0, "gcl::test::event::handler : bad B_event::counter value");
					handler->on(gcl::deprecated::type_info::id<B_event>::value, *ev);
					GCL_TEST__EXPECT(B_event::counter == 1, "gcl::test::event::handler : bad B_event::counter value");

					gcl::deprecated::type_info::holder<gcl_event_t::interface_t> event_value_holder(new C_event());
					GCL_TEST__EXPECT(C_event::counter == 0, "gcl::test::event::handler : bad C_event::counter value");
					handler->on(event_value_holder);
					GCL_TEST__EXPECT(C_event::counter == 1, "gcl::test::event::handler : bad C_event::counter value");
				}
			};
			struct dispatcher
			{
				static void proceed()
				{
					gcl_event_t::dispatcher dispatcher;

					using handler_t = gcl_event_t::handler;
					using event_t = gcl_event_t::interface_t;

					std::shared_ptr<handler_t> handler1(new gcl_event_t::many_to_one_handler());
					std::shared_ptr<handler_t> handler2(new gcl_event_t::many_to_many_handler());

					/*std::cout
						<< gcl::type_info::id<gcl_event_t::interface_t>::value << " => gcl::type_info::id<gcl_event_t::interface_t>::value" << std::endl
						<< gcl::type_info::id<A_event>::value << " => gcl::type_info::id<A_event>::value" << std::endl
						;*/

					handler1->add_listener<A_event>(std::move([](const event_t &) { /*std::cout << "A_event -> handler1 : many_to_one_handler : 1 : " << static_cast<const A_event &>(ev).a_var << std::endl;*/ }));
					/*handler1->add_listener<A_event>(std::move([](const event_t & ev) { std::cout << "A_event -> handler2 : many_to_many_handler  : 1 : " << static_cast<const A_event &>(ev).a_var << std::endl; }));
					handler1->add_listener<A_event>(std::move([](const event_t & ev) { std::cout << "A_event -> handler2 : many_to_many_handler  : 2 : " << static_cast<const A_event &>(ev).a_var << std::endl; }));
					handler1->add_listener<B_event>(std::move([](const event_t & ev) { std::cout << "B_event -> handler2 : many_to_many_handler  : 3 : " << static_cast<const B_event &>(ev).b_var << std::endl; }));*/

					dispatcher.subscribe<A_event>(handler1);
					//dispatcher.subscribe<A_event>(handler2);

					dispatcher.dispatch(A_event{ 42 });
					/*std::cout << "dispatching B_event :" << std::endl;
					dispatcher.dispatch(B_event{ "Hello, world" });
					std::cout << "Remove handler1" << std::endl;
					dispatcher.unsubscribe<A_event>(handler1);
					std::cout << "dispatching A_event :" << std::endl;
					dispatcher.dispatch(A_event{ 42 });*/
				}
			};
			struct experimental
			{
				struct static_socket
				{
					static void proceed()
					{
						struct Event_A {};
						struct Event_B {};
						struct Event_C {};

						{	// event_socket
							gcl_event_t::experimental::static_socket
							<
								Event_A,
								Event_B,
								Event_C
							> myEventHandler;

							struct
							{
								std::size_t
									a{ 0 },
									b{ 0 },
									c{ 0 };
							} _cb_call_counters1, _cb_call_counters2;

							myEventHandler.add<Event_A>([&]() { _cb_call_counters1.a++;	});
							myEventHandler.add<Event_B>([&]() { _cb_call_counters1.b++;	});
							myEventHandler.add<Event_C>([&]() { _cb_call_counters1.c++;	});

							myEventHandler.add<Event_A>([&]() { _cb_call_counters2.a++;	});
							myEventHandler.add<Event_B>([&]() { _cb_call_counters2.b++;	});
							myEventHandler.add<Event_C>([&]() { _cb_call_counters2.c++;	});

							myEventHandler.trigger<0>();		// Event_A by index
							myEventHandler.trigger<Event_B>();	// Event_B by static type
							myEventHandler.trigger(Event_C{});	// Event_C by dynamic type

							GCL_TEST__EXPECT(_cb_call_counters1.a == 1, "gcl::test::event::experimental::static_socket : bad A_event counter value");
							GCL_TEST__EXPECT(_cb_call_counters1.b == 1, "gcl::test::event::experimental::static_socket : bad B_event counter value");
							GCL_TEST__EXPECT(_cb_call_counters1.c == 1, "gcl::test::event::experimental::static_socket : bad C_event counter value");
							GCL_TEST__EXPECT(_cb_call_counters2.a == 1, "gcl::test::event::experimental::static_socket : bad A_event counter value");
							GCL_TEST__EXPECT(_cb_call_counters2.b == 1, "gcl::test::event::experimental::static_socket : bad B_event counter value");
							GCL_TEST__EXPECT(_cb_call_counters2.c == 1, "gcl::test::event::experimental::static_socket : bad C_event counter value");
						}
					}
				};

				using dependencies_t = std::tuple<static_socket>;
			};

			struct route {};

			using dependencies_t = std::tuple<handler, dispatcher, experimental, route>;
		};
	}
}

std::size_t gcl::test::event::A_event::counter = 0;
std::size_t gcl::test::event::B_event::counter = 0;
std::size_t gcl::test::event::C_event::counter = 0;