#ifndef GCL_FILESYSTEM_HPP__
# define GCL_FILESYSTEM_HPP__

# include <stdexcept>
# include <functional>
# include <vector>

#ifdef _WIN32
# include <Windows.h>
# include <strsafe.h>
#endif

namespace gcl
{
	namespace filesystem
	{
		namespace exception
		{
			struct bad_path : std::runtime_error
			{
				bad_path(const std::string & path)
					: std::runtime_error("gcl::fs : Bad path : [" + path + "]")
				{}
			};
			struct walk_error : std::runtime_error
			{
				walk_error(const std::string & path)
					: std::runtime_error("gcl::fs : walk error : [" + path + "]")
				{}
			};
		}

		struct path
		{
			path(const std::string & str)
				: value{ str }
			{}
			path(const path & other)
				: value(other)
				, is_valid(other.is_valid)
			{}
			path(path && other)
				: value(std::move(other.value))
				, is_valid(std::move(other.is_valid))
			{}

			inline operator const std::string &() const
			{
				return value;
			}
			inline operator const char *() const
			{
				return value.c_str();
			}
			inline bool operator!() const
			{
				if (is_valid)
					return true;

				std::size_t length;
				StringCchLength(value.c_str(), MAX_PATH, &length);
				return (const_cast<path*>(this)->is_valid = (length > (MAX_PATH - 3)));
			}

			inline path operator+(const std::string & str) const
			{
				return path{ value + str };
			}
			inline path & operator=(const std::string & str)
			{
				value = str;
				is_valid = false;

				return *this;
			}

		protected:
			std::string value;
			bool is_valid = false;
		};

		struct directory
		{
			enum walk_logic
			{
				non_recursive,
				recursive
			};

			directory(const std::string & path_v)
				: path(path_v)
			{}

			template <walk_logic is_rec = non_recursive>
			std::vector<std::string> directories()
			{
				std::vector<std::string> dirlist;

				walk_routine_t fun = [&dirlist](const std::string & filename, bool is_dir)
				{
					if (is_dir)
						dirlist.push_back(filename);
				};
				walk<is_rec>(path, fun);

				return dirlist;
			}
			template <walk_logic is_rec = non_recursive>
			std::vector<std::string> files()
			{
				std::vector<std::string> filelist;

				auto fun = [&filelist](const std::string & filename, bool is_dir)
				{
					if (!is_dir)
						filelist.push_back(filename);
				};
				walk<is_rec>(path, fun);

				return filelist;
			}
			template <walk_logic is_rec = non_recursive>
			std::vector<std::string> content()
			{
				std::vector<std::string> contentlist;

				auto fun = [&contentlist](const std::string & filename, bool is_dir)
				{
					contentlist.push_back(filename);
				};
				walk<is_rec>(path, fun);

				return contentlist;
			}

			using walk_routine_t = std::function<void(const std::string & /*filename*/, bool /*is_dir*/)>;
			template <walk_logic is_rec = non_recursive>
			void walk(const filesystem::path & path_value, walk_routine_t fun)
			{
				walk_impl<is_rec>(path_value, fun);
			}

		protected:
			filesystem::path path;

#ifdef WIN32
			template <walk_logic is_rec>
			void walk_impl(const filesystem::path & path_value, walk_routine_t fun)
			{
				if (!path_value)
					throw exception::bad_path{ path_value };

				HANDLE handle{ INVALID_HANDLE_VALUE };

				TCHAR cache[MAX_PATH];
				StringCchCopy(cache, MAX_PATH, path_value);
				StringCchCat(cache, MAX_PATH, TEXT("\\*"));

				WIN32_FIND_DATA find_data;
				bool next_success{ true };

				for (handle = FindFirstFile(cache, &find_data);
					handle != INVALID_HANDLE_VALUE && next_success;
					next_success = (FindNextFile(handle, &find_data) != 0))
				{
					if (strcmp(find_data.cFileName, ".") != 0 &&
						strcmp(find_data.cFileName, "..") != 0)
					{
						const bool is_dir = find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
						auto file_path = path_value + "/" + find_data.cFileName;
						fun(file_path, is_dir);
						if (is_dir && is_rec)
							walk_impl<is_rec>(file_path, fun);
					}
				}
				if (GetLastError() != ERROR_NO_MORE_FILES)
					throw exception::walk_error(path_value);

			}
#else
			static_assert(false, "gcl::filesystem::directory::walk : not implemented");
			template <walk_logic is_rec>
			void walk_impl(const filesystem::path & path_value, walk_routine_t fun)
			{}
#endif
		};
	}
}

#endif // GCL_FILESYSTEM_HPP__