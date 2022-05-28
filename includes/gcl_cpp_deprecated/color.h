#ifndef GCL_COLOR_H_
# define GCL_COLOR_H_

#include <iosfwd>
#include <iostream>
#include <iomanip>
#ifdef _WIN32
# include <Windows.h>
#endif

// todo :  operator "" overload, to get a nodejs npm color synthax-like

namespace gcl::color
{
	enum class values : unsigned short
	{
		black = 0,
		dark_blue = 1,
		dark_green = 2,
		dark_aqua, dark_cyan = 3,
		dark_red = 4,
		dark_purple = 5, dark_pink = 5, dark_magenta = 5,
		dark_yellow = 6,
		dark_white = 7,
		gray = 8,
		blue = 9,
		green = 10,
		aqua = 11, cyan = 11,
		red = 12,
		purple = 13, pink = 13, magenta = 13,
		yellow = 14,
		white = 15
	};

#ifdef _WIN32
	struct set
	{
		concol _foreground;
		concol _background;

		friend std::ostream & operator<<(std::ostream &, const set &);
		friend std::istream & operator>>(std::istream &, const set &);
	};
	struct default_console_set_type : set
	{
		HANDLE std_con_out;

		explicit default_console_set_type()
		{
			Init();
		}
		default_console_set_type(const default_console_set_type &) = delete;
		default_console_set_type(const default_console_set_type &&) = delete;
		default_console_set_type & operator=(const default_console_set_type &) = delete;

	protected:
		void    Init(void)
		{
			std_con_out = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_SCREEN_BUFFER_INFO consoleScreen_bufferInfo;
			GetConsoleScreenBufferInfo(std_con_out, &consoleScreen_bufferInfo);

			_foreground = concol(consoleScreen_bufferInfo.wAttributes & 15);
			_background = concol((consoleScreen_bufferInfo.wAttributes & 0xf0) >> 4);
		}
	} static const default_console_set;// = set{ concol::white, concol::black };

	std::ostream & operator<<(std::ostream & os, const set & set)
	{
		os.flush();
		unsigned short attr = ((static_cast<unsigned short>(set._background) << 4) | static_cast<unsigned short>(set._foreground));
		SetConsoleTextAttribute(default_console_set.std_con_out, attr);
		return os;
	}
	std::istream & operator>>(std::istream & is, const set & set)
	{
		unsigned short attr = ((static_cast<unsigned short>(set._background) << 4) | static_cast<unsigned short>(set._foreground));
		SetConsoleTextAttribute(default_console_set.std_con_out, attr);
		return is;
	}

#elif defined (GNU)
# error "Not implemented yet"
#else
# error "Not implemented yet"
#endif
}

// todo : user literal

#endif // GCL_COLOR_H_
