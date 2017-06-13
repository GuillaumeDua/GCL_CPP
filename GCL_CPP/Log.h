#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <iomanip>
#include <vector>
#include <functional>
#include <unordered_set>
#include <cctype>
#include <functional>
#include <sstream>
#include <set>
#include <fstream>

/*
auto binded = std::bind(myLambdaWithIntParam, 55);
binded();
// std::bind1st(std::mem_fun_ref(&MyPredicate_base::Compare), pb);
*/

namespace Log
{
	typedef std::ostream& (*T_SteamManipulator)(std::ostream & os);

	enum struct ChannelName : char
	{
		STD_OUT
		, STD_WOUT
		, STD_ERR
		, STD_WERR
		, STD_LOG
		, STD_WLOG
		// , ...
	};

	// todo : refactor using stream binding
	struct Channel
	{
		Channel() = default;

		explicit Channel(std::ostream & os)
		{
			_records.insert(&os);
		}
		explicit Channel(std::vector<std::ostream *> streams)
			: _records(streams.cbegin(), streams.cend())
		{}

		Channel & operator+=(std::ostream & os)
		{
			_records.insert(&os);
			return *this;
		}
		template <typename T>
		Channel & operator<<(const T & data)
		{
			for (auto & elem : _records)
				*elem << data;
			return *this;
		}
		Channel operator << (T_SteamManipulator manipulator)
		{
			for (auto & elem : _records)
				manipulator(*elem);
			return *this;
		}

		std::set<std::ostream *>	_records;
	};

	struct	System
	{
		using T_ChannelContainer = std::unordered_map<ChannelName, Channel>;

		/*void	AddChannel(T_ChannelContainer::value_type && channel)
		{
			_channels.insert(channel);
		}*/
		/*ChannelName	CreateNewChannel()
		{
			ChannelName maxChannelId(0);

			for (auto & elem : _channels)
				maxChannelId = (maxChannelId < elem.first ? elem.first : maxChannelId);
			_channels[maxChannelId];
			return maxChannelId;
		}*/

		template <typename T>
		System & operator<<(const T & data)
		{
			for (auto & elem : _channels)
				elem.second << data;
		}

		Channel & operator[](const ChannelName channel_id)
		{
			if (_channels.find(channel_id) == _channels.cend())
				throw std::runtime_error("Channel : no registered channel to this ID");
			return _channels[channel_id];
		}

		T_ChannelContainer	_channels =
			T_ChannelContainer({
				{ static_cast<ChannelName>(ChannelName::STD_OUT), Channel{ std::cout } }
			,	{ static_cast<ChannelName>(ChannelName::STD_ERR), Channel{ std::cerr } }
			,	{ static_cast<ChannelName>(ChannelName::STD_LOG), Channel{ std::clog } }
		});
	};
	
	namespace Test
	{
		void	Process()
		{
			try
			{
				Log::System logSystem;

				logSystem[Log::ChannelName::STD_ERR] << "This is the first error\n";// << std::endl;

				std::ofstream ofsErrorLogFile("Errors.log");

				logSystem[Log::ChannelName::STD_ERR] += ofsErrorLogFile;

				logSystem[Log::ChannelName::STD_ERR] << "This is the second error\n";
			}
			catch (const std::exception & ex)
			{
				std::cerr << "[Error] : Exception catch : " << ex.what() << std::endl;
			}
		}
	}
}
