#ifndef NL_HPP
#define NL_HPP

#define NL_VERSION 1.0

#include "listener.hpp"
#include "notifier.hpp"
#include "trackable.hpp"

namespace nl{
template<typename TFunc> struct functor;

template<typename TReturn, typename... TParams> struct functor<TReturn (TParams...)>{
	typedef listener<TReturn (TParams...)> listener_type;
	typedef notifier<TReturn (TParams...)> notifier_type;

	typedef typename notifier_type::connection connection_type;
};

}

#endif
