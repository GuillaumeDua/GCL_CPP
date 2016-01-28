#ifndef __OBJ_BINDING__
# define __OBJ_BINDING__

# include <vector>

namespace GCL // Guss Common Lib
{
	// [Warning] : Definitly not thread-safe
	template <class T> struct Linkable
	{
		Linkable()
			: _LinkedWith(0x0)
		{}
		Linkable(T * toLinkWith)
			: _LinkedWith(toLinkWith)
		{}

		void	LinkWith(T * toLinkWith)
		{
			_LinkedWith = toLinkWith;
		}

	protected:
		T * _LinkedWith;
	};

	struct Bindable
	{
		Bindable()
			: _bindedWith(0x0)
		{}

		Bindable(Bindable * toBindWith)
			: _bindedWith(toBindWith)
		{
			toBindWith->SetBind(this);
		}

	protected:
		void	SetBind(Bindable * toBindWith)
		{
			this->_bindedWith = toBindWith;
		}

		Bindable * _bindedWith;
	};
}

#endif // __OBJ_BINDING