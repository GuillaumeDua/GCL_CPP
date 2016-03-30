#ifndef __GCL_OWNERSHIP__
# define __GCL_OWNERSHIP__

# include <set>
# include <map>
# include <unordered_map>
# include <unordered_set>
# include <vector>

namespace GCL
{
	/*
	elem.on(Notification::DTOR_EVENT_NAME).emplace_back(std::move(OnDestructionCalledStdFunction([this, &elem]()
	{
	this->remove(elem);
	})));
	*/

	namespace Ownership
	{
		template <typename T>
		struct Owner
		{
		protected:
			// 1 owner -> n ownees
			struct OwnershipRecorder
			{
			protected:

				using T_OwnersToOwnees = std::unordered_map < const Owner<T> *, std::unordered_set<const T *> > ;
				T_OwnersToOwnees	_ownersToOwnees;

				using T_OwneesToOwnersMap = std::unordered_map < const T *, const Owner<T> * > ;
				T_OwneesToOwnersMap	_owneesToOwners;

			public:
				bool											Add(const Owner<T> & owner, const T & ownee)
				{
					try
					{
						return (this->GetOwner(ownee) == &owner);
					}
					catch (const std::out_of_range &)
					{
						_ownersToOwnees[&owner].insert(&ownee);
						_owneesToOwners[&ownee] = &owner;
						return true;
					}
				}
				void											Remove(const Owner<T> & owner)
				{
					for (auto elem : _ownersToOwnees[&owner])
						_owneesToOwners.erase(elem);
					_ownersToOwnees.erase(&owner);
				}
				void											Remove(const T & ownee)
				{
					auto matchIt = _owneesToOwners.find(&ownee);
					if (matchIt == _owneesToOwners.cend())
						return;
					this->Remove(matchIt->second);
				}

				const Owner<T> *								GetOwner(const T & elem)	// std::out_of_range
				{
					return _owneesToOwners.at(&elem);
				}
				typename const T_OwneesToOwnersMap::mapped_type	GetOwnees(const Owner<T> & owner)
				{
					return _ownersToOwnees.at(&owner);
				}

			} static _ownershipRecorder;

		public:
			~Owner()
			{
				_ownershipRecorder.Remove(*this);
			}

			bool	HasOwnership(const T & var) const
			{
				try
				{
					return (_ownershipRecorder.GetOwner(var) == &this);
				}
				catch (const std::out_of_range &)
				{
				}

				return false;
			}
			bool	TryAcquireOwnership(const T & var) const
			{
				return _ownershipRecorder.Add(*this, var);
			}
			bool	Release(void) const
			{
				_ownershipRecorder.Remove(*this);
			}
			bool	Release(const T & var) const
			{
				try
				{
					if (_ownershipRecorder.GetOwner(var) != &this)
						throw std::runtime_error("Ownership release failed : Wrong owner");
					_ownershipRecorder.Remove(var);
				}
				catch (const std::out_of_range &)
				{
					throw std::runtime_error("Ownership release failed : record is missing");
				}
			}
		};

		template <typename T>
		typename Owner<T>::OwnershipRecorder Owner<T>::_ownershipRecorder;

		struct Test
		{
			struct Cow
			{
				explicit Cow(const std::string & name)
					: _name(name)
				{}

				std::string _name;
			};
			struct Farmer : public Owner < Cow >
			{};

			static bool	Proceed(void)
			{
				std::vector<Cow> cows = { Cow("Jeannette"), Cow("Fleurette"), Cow("Paulette"), Cow("Mirabelle") };

				Farmer Gerard;

				if (!Gerard.TryAcquireOwnership(cows.at(0))
					|| !Gerard.TryAcquireOwnership(cows.at(1)))
					return false;

				{
					Farmer Fabrice;

					if (!Fabrice.TryAcquireOwnership(cows.at(2))
						|| !Fabrice.TryAcquireOwnership(cows.at(3)))
						return false;

					if (Fabrice.TryAcquireOwnership(cows.at(0)))
						return false;
				}

				if (!Gerard.TryAcquireOwnership(cows.at(2)))
					return false;

				return true;
			}
		};
	}
}

#endif // __GCL_OWNERSHIP__