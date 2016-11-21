#ifndef GCL_RUNTIMECONTROLER_H__
# define GCL_RUNTIMECONTROLER_H__

# include <string>
# include <functional>
# include <list>
# include <unordered_map>
# include <memory>
# include <iostream>

# include "Preprocessor.h"

namespace GCL
{
	namespace Experimental
	{
		namespace RunTime
		{
			struct ExpressionControler
			{
				struct Observer
				{
					virtual void	NotifyPropertyChanged(void) = 0;
				};

				using T_KeyHash = std::string;	// todo : key_type, mapped_type, iterator : STL compatibility
				struct T_HookPoint
				{
					using T_Callback = std::function<void()>;

					T_HookPoint(T_Callback && cb)
						: _conditionnalSnippet(cb)
					{}

					T_Callback	_conditionnalSnippet = nullptr;
					bool isActive = false;
					// todo : std::string description : callback code to string
				};
				using T_ObserversList = std::list<std::shared_ptr<Observer>>;

			private:
				using T_Container = std::unordered_map<T_KeyHash, T_HookPoint>;
				static T_Container _container;
				static T_ObserversList	_observers;

			public:

				static void								OnPropertyChanged(void)
				{
					for (auto & obs : _observers)
						obs->NotifyPropertyChanged();
				}
				static const T_Container::mapped_type &	Insert(T_Container::value_type && value)
				{
					T_Container::iterator it = _container.find(value.first);
					if (it == _container.end())
					{
						auto insert_ret = _container.emplace(value);
						_GCL_ASSERT(insert_ret.second);
						it = insert_ret.first;
						OnPropertyChanged();
					}
					return it->second;
				}

				static inline void	Activate(const size_t distance)
				{
					T_Container::iterator it = _container.begin();
					std::advance(it, distance);
					Activate(it);
				}
				static inline void	Activate(const T_KeyHash key)
				{
					Activate(_container.find(key));
				}
				static inline void	Activate(T_Container::iterator & it)
				{
					it->second.isActive = true;
					// std::cout << "[+]::[INFO] : Activating hookpoint at : " << it->first << std::endl;
				}
			};

			//ExpressionControler::T_Container ExpressionControler::_container;
			//ExpressionControler::T_ObserversList ExpressionControler::_observers;
		}
	}
}

//#define CONDITIONAL_SNIPPET(expr)	static const ExpressionControler::T_KeyHash key = std::string(__FILE__ " line ") + std::to_string(__LINE__);	\
//									ExpressionControler::T_Container::iterator it = ExpressionControler::_container.find(key);				\
//									if (it == ExpressionControler::_container.cend())															\
//									ExpressionControler::_container.insert(std::make_pair(key, std::move([&]() { expr })));						\
//									else if (it->second.isActive)																					\
//									it->second._conditionnalSnippet();

#define CONDITIONAL_SNIPPET(expr)	static const GCL::Experimental::RunTime::ExpressionControler::T_KeyHash key = std::string(__FILE__ " line ") + std::to_string(__LINE__);																				\
									const GCL::Experimental::RunTime::ExpressionControler::T_HookPoint & hook =																																				\
										GCL::Experimental::RunTime::ExpressionControler::Insert(std::pair<GCL::Experimental::RunTime::ExpressionControler::T_KeyHash, GCL::Experimental::RunTime::ExpressionControler::T_HookPoint>(key, [&]() { expr }));	\
									if (hook.isActive)																																																		\
										hook._conditionnalSnippet();

#endif // GCL_RUNTIMECONTROLER_H__