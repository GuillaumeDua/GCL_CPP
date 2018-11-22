#ifndef GCL_OLD__MONITORING_HPP__
# define GCL_OLD__MONITORING_HPP__

# error this file is deprecated (too old / unstable) and will be delete shortly

# include <gcl_cpp/pattern.hpp>
# include <list>
# include <cstdint>
# include <mutex>
# include <cassert>
# include <map>

namespace gcl
{
	namespace old
	{
		// state-machine
		struct Runnable
		{
			// odd	: Transition state
			// even	: Stable state
			enum class Status : uint8_t
			{
				StandBy
				, Starting
				, Started
				, Stopping
				, Stopped
			};

			void				RequestStatusChange(const Status status)
			{
				assert(uint8_t(status) % uint8_t(1) == 0); // "Status must be odd (transition stat)"
				// TODO
			}

			inline const Status GetStatus(void) // const
			{
				{	// critical section
					std::unique_lock<std::mutex> lock(_mutex);
					return _status;
				}
			}
			static const bool	IsStatusChangeValid(const Status current, const Status requested)
			{
				const std::list<Status> & list = _statusChangeValidator.at(current);
				return (std::find(list.begin(), list.end(), current) != list.cend());
			}

		protected:
			inline void SetStatus(const Status status)
			{
				assert(IsStatusChangeValid(_status, status));
				{	// critical section
					std::unique_lock<std::mutex> lock(_mutex);
					_status = status;
				}
			}

			static const std::map<Status, std::list<Status> > _statusChangeValidator;

			Status		_status;
			std::mutex	_mutex;
		};
		// TODO : Move to .cpp file
		const std::map<Runnable::Status, std::list<Runnable::Status> > Runnable::_statusChangeValidator =
		{
				{ Runnable::Status::StandBy	,	{ Starting }}
			,	{ Runnable::Status::Starting,	{ Started, Stopping }}
			,	{ Runnable::Status::Started	,	{ Stopping }}
			,	{ Runnable::Status::Stopping,	{ Stopped }}
			,	{ Runnable::Status::Stopped	,	{ StandBy, Starting }}
		};

		struct Readyness
		{
			using T_FilterList = std::list<std::shared_ptr<Runnable>>;

			void		WaitUntilReady(const Runnable::Status status, std::function<void()> onReadyCB)
			{
				// TODO : Use condition variable

				for (auto & module : _modules)
					if (module->GetStatus() != status										// ex : Starting
						&& uint8_t(module->GetStatus()) != (uint8_t(status) + uint8_t(1)))	//    : Started
						; // no ready yet
				onReadyCB();
			}

			inline void	AddModule(std::shared_ptr<T_FilterList> & filter)
			{
				_modules.push(filter);
			}

			void		RequestStatus(const Runnable::Status status)
			{
				assert(uint8_t(status) % uint8_t(1) == 0); // "Status must be odd (transition stat)"
				for (auto & module : _modules)
					module->RequestStatusChange(status);
			}
			/*void		BroadcastStatus(const Runnable::Status status)
			{
				for (auto & module : _modules)
					module->SetStatus(status);
			}*/
			bool		checkModulesStatus(const Runnable::Status status)
			{
				for (auto & module : _modules)
					if (module->GetStatus() != status)
						return false;
				return true;
			}

		protected:
			T_FilterList	_modules;
		};
	}
}

#endif // GCL_OLD__MONITORING_HPP__