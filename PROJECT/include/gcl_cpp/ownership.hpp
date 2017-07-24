#ifndef GCL_OWNERSHIP__HPP__
# define GCL_OWNERSHIP__HPP__

# include <set>
# include <map>
# include <unordered_map>
# include <unordered_set>
# include <vector>

namespace gcl
{
	namespace ownership
	{
		template <typename T>
		struct owner;

		// 1 owner -> n ownees
		template <typename T>
		struct recorder
		{
			using element_type = T;

		protected:
			using map_owner_to_ownees_t = std::unordered_map < const owner<T> *, std::unordered_set<const T *> >;
			map_owner_to_ownees_t	_ownersToOwnees;

			using map_ownees_to_owner_t = std::unordered_map < const T *, const owner<T> * >;
			map_ownees_to_owner_t	_owneesToOwners;

		public:
			bool											add(const owner<T> & owner, const T & ownee)
			{
				try
				{
					return (&(this->get_owner(ownee)) == &owner);
				}
				catch (const std::out_of_range &)
				{
					_ownersToOwnees[&owner].insert(&ownee);
					_owneesToOwners[&ownee] = &owner;
					return true;
				}
			}
			void											remove(const owner<T> & owner)
			{
				for (auto elem : _ownersToOwnees[&owner])
					_owneesToOwners.erase(elem);
				_ownersToOwnees.erase(&owner);
			}
			void											remove(const T & ownee)
			{
				auto matchIt = _owneesToOwners.find(&ownee);
				if (matchIt == _owneesToOwners.cend())
					return;
				this->Remove(matchIt->second);
			}

			const owner<T> &								get_owner(const T & elem)	// std::out_of_range
			{
				return *(_owneesToOwners.at(&elem));
			}
			typename const map_ownees_to_owner_t::mapped_type &	get_ownees(const owner<T> & owner)
			{
				return _ownersToOwnees.at(&owner);
			}

		};

		template <typename T>
		struct owner
		{
			using element_type = T;

		protected:
			static recorder<element_type> _ownershipRecorder;
		public:
			~owner()
			{
				_ownershipRecorder.remove(*this);
			}

			bool	has(const element_type & var) const
			{
				try
				{
					return (_ownershipRecorder.get_owner(var) == &this);
				}
				catch (const std::out_of_range &)
				{
					return false;
				}
			}
			bool	try_acquire(const element_type & var) const
			{
				return _ownershipRecorder.add(*this, var);
			}
			bool	release(void) const
			{
				_ownershipRecorder.remove(*this);
			}
			bool	release(const element_type & var) const
			{
				try
				{
					if (_ownershipRecorder.get_owner(var) != &this)
						throw std::runtime_error("ownership release failed : Wrong owner");
					_ownershipRecorder.Remove(var);
				}
				catch (const std::out_of_range &)
				{
					throw std::runtime_error("ownership release failed : record is missing");
				}
			}
		};

		template <typename T>
		typename recorder<T> owner<T>::_ownershipRecorder;

		struct Test
		{
			struct Cow
			{
				explicit Cow(const std::string & name)
					: _name(name)
				{}

				std::string _name;
			};
			struct Farmer : public owner < Cow >
			{};

			static bool	Proceed(void)
			{
				std::vector<Cow> cows = { Cow("Jeannette"), Cow("Fleurette"), Cow("Paulette"), Cow("Mirabelle") };

				Farmer Gerard;

				if (!Gerard.try_acquire(cows.at(0))
					|| !Gerard.try_acquire(cows.at(1)))
					return false;

				{
					Farmer Fabrice;

					if (!Fabrice.try_acquire(cows.at(2))
						|| !Fabrice.try_acquire(cows.at(3)))
						return false;

					if (Fabrice.try_acquire(cows.at(0)))
						return false;
				}

				if (!Gerard.try_acquire(cows.at(2)))
					return false;

				return true;
			}
		};
	}
}

#endif // GCL_OWNERSHIP__HPP__