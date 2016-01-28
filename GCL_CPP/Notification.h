#ifndef __GCL_NOTIFICATION__
# define __GCL_NOTIFICATION__

# include "./Vector.h"

# include <map>
# include <queue>
# include <functional>
# include <iostream>

namespace GCL
{
	namespace Notification
	{
		// Warning : Make sure that Notifications's callbacks captured variable have life-length that are bigger than this object.
		// [Todo] : Replace std::string by template parameter
		template <typename T_EventIDType = std::string>
		struct Notifiable
		{
			using T_EventID = typename T_EventIDType;
			using EventContainer = typename std::map<typename T_EventID, GCL::Vector<std::function<void()> > >;

			typename EventContainer::mapped_type &	on(const T_EventID & name)
			{
				return _events[name];
			}
			void							Notify(const T_EventID & name)
			{
				_pendingStatus.push(name);
			}
			void							ResolvePendingNotifications(void)
			{
				while (!_pendingStatus.empty())
				{
					TriggerEvent(_pendingStatus.front());
					_pendingStatus.pop();
				}
			}
			void							TriggerEvent(const T_EventID & name)
			{
				if (_events.find(name) == _events.end())
					return;

				for (auto & elem : _events.at(name))
					elem();
			}

		protected:
			EventContainer					_events;
			std::queue<T_EventID>			_pendingStatus;
		};

		namespace Test
		{
			struct Toto : public Notifiable<>
			{
				void	DoStuff(void)
				{
					std::cout << "Toto::DoStuff" << std::endl;
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
					std::cout << "[0x" << this << "] : CollisionNotificationLogger::Report : " << i << std::endl;
				}
			};

			void	Process()
			{
				Toto toto;
				toto.on("Collision") += [](){ std::cout << "Collision events triggered" << std::endl; };
				toto.on("Collision") += [&toto](){ toto.DoStuff(); };
				toto.on("Collision") += [&toto]()  { toto.IncrI(); };
				toto.on("Collision") += [toto]()  mutable { toto.IncrI(); };
				// toto.on("Collision") += [toto]()   { toto.IncrI(); };

				CollisionNotificationLogger collisionLogger;
				toto.on("Collision") += [toto, &collisionLogger]() mutable { collisionLogger.Report(toto._i); };
				toto.on("Collision") += [&toto, &collisionLogger]() { collisionLogger.Report(toto._i); };
				toto.TriggerEvent("Collision");

				toto.TriggerEvent("Collision");
			}
		}
	}
}

#endif // __GCL_NOTIFICATION__