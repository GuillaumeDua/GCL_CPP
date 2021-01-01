#pragma once

#include <csignal>
#include <vector>
#include <string>
#include <iostream>

namespace gcl::signals
{
	static inline const std::unordered_map<int, std::string> values =
	{
		{ SIGABRT, "SIGABRT : Abnormal termination of the program, such as a call to abort." },
		{ SIGFPE, "SIGFPE : An erroneous arithmetic operation, such as a divide by zero or an operation resulting in overflow." },
		{ SIGILL, "SIGILL : Detection of an illegal instruction. " },
		{ SIGINT, "SIGINT : Receipt of an interactive attention signal." },
		{ SIGSEGV, "SIGSEGV : An invalid access to storage." },
		{ SIGTERM, "SIGTERM" }
	};

	static void handler(int signum)
	{
		try
		{
			std::cerr << "error : signal catch : " << values.at(signum) << std::endl;
		}
		catch (const std::exception &)
		{
			std::cerr << "error : cannot examinate signals : " << signum << std::endl;
		}

#ifdef _WIN32
		system("pause");
#endif
		exit(signum);
	}
	static void initialize()
	{
		for (const auto & value : values)
		{
			signal(value.first, handler);
		}
	}
}