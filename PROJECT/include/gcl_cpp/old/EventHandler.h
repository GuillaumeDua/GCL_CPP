#ifndef GCL_OLD__EVENT_HANDLER_HPP__
# define GCL_OLD__EVENT_HANDLER_HPP__

# error this file is deprecated (too old / unstable) and will be delete shortly

# include <gcl_cpp/Container.h>
# include <gcl_cpp/preprocessor.hpp>

# include <map>
# include <queue>
# include <functional>
# include <iostream>

namespace gcl
{
	namespace old
	{
		// deprecated : use gcl_cpp/event.hpp instead
		namespace events
		{
			// todo : rename as Handler
			//        avoid repetition in naming
			template <typename T_EventIDType = std::string>
			struct EventHandler
			{
				explicit EventHandler() = default;
				EventHandler(EventHandler && obj)
					: _events(std::move(obj._events))
					, _pendingStatus(std::move(obj._pendingStatus))
				{}
				virtual ~EventHandler() {}
				EventHandler & operator=(EventHandler && obj)
				{
					_events = std::move(obj._events);
					_pendingStatus = std::move(obj._pendingStatus);
					return *this;
				}

				using T_EventID = typename T_EventIDType;
				using T_EventCallback = std::function<void()>;
				using T_EventContainer = typename std::map<typename T_EventID, gcl::container::non_concurrent::Vector<typename T_EventCallback> >;

				typename T_EventContainer::mapped_type &	on(const T_EventID & name)
				{
					return _events[name];
				}
				void										Notify(const T_EventID & name)
				{
					_pendingStatus.push(name);
				}
				void										TriggerPendingEvents(void)
				{
					while (!_pendingStatus.empty())
					{
						TriggerEvent(_pendingStatus.front());
						_pendingStatus.pop();
					}
				}
				void										TriggerEvent(const T_EventID & name)
				{
					if (_events.find(name) == _events.end())
						return;

					for (auto & elem : _events.at(name))
						elem();
				}

			protected:
				T_EventContainer				_events;
				std::queue<T_EventID>			_pendingStatus;
			};

			struct Test
			{
				struct Toto : public EventHandler<>
				{
					void	DoStuff(void)
					{
						GCL_DEBUG_INSTRUCTION(std::cout << "Toto::DoStuff" << std::endl);
					}
					void	IncrI(void)
					{
						++_i;
					}

					int _i = 0;
				};

				struct CollisionNotificationLogger
				{
					void	Report(const int i)
					{
						GCL_DEBUG_INSTRUCTION(std::cout << "[0x" << this << "] : CollisionNotificationLogger::Report : " << i << std::endl);
					}
				};

				static bool	Proceed()
				{
					Toto toto;
					toto.on("Collision") += []() { GCL_DEBUG_INSTRUCTION(std::cout << "Collision events triggered" << std::endl); };
					toto.on("Collision") += [&toto]() { toto.DoStuff(); };
					toto.on("Collision") += [&toto]() { toto.IncrI(); };
					toto.on("Collision") += [&toto]()  mutable { toto.IncrI(); };
					// toto.on("Collision") += [toto]()   { toto.IncrI(); };

					CollisionNotificationLogger collisionLogger;
					toto.on("Collision") += [&toto, &collisionLogger]() mutable { collisionLogger.Report(toto._i); };
					toto.on("Collision") += [&toto, &collisionLogger]() { collisionLogger.Report(toto._i); };
					toto.TriggerEvent("Collision");

					toto.TriggerEvent("Collision");

					return true;
				}
			};
		}
	}
}

#endif // GCL_OLD__EVENT_HANDLER_HPP__