#ifndef NL_LISTENER_HPP
#define NL_LISTENER_HPP

#include "trackable.hpp"

namespace nl{
template<typename TFunc> class listener;

///@class listener
///@brief Listener class for invoking a target function
///@details Class for storing a target function
///(static or pointer-to-member) for later invocation.
///@remarks Listeners that target a pointer-to-member
///function are only valid as long as the associated
///object exists. Invoking them beyond the lifetime
///of the object may cause undefined behavior (unless)
///they're tracked by a nl::trackable-derived object).
///@ingroup listener
template<typename TReturn, typename... TParams> class listener<TReturn (TParams...)>{
public:
	typedef listener<TReturn (TParams...)> this_type;

	typedef TReturn return_type;
	typedef return_type (*func_type)(TParams...);

	///@brief Method for constructing a listener object
	///@details Method for constructing a listener
	///object with a pointer to the specified static
	///function.
	///@return Listener object
	///@static
	template<return_type (*TFunc)(TParams...)> static inline auto make()noexcept -> const this_type {
		return (this_type (&function_stub<TFunc>));
	}

	///@brief Methods for constructing a listener object
	///@details Methods for constructing a listener
	///object with a pointer to the specified
	///pointer-to-member function.
	///@return Listener object
	///@static
	template<typename TObj, return_type (TObj::*TMethod)(TParams...)>
			static inline auto make(TObj* obj)noexcept -> const this_type {
		return (this_type (&method_stub<TObj, TMethod>, obj, detail::is_trackable<TObj>::value));
	}

	template<typename TObj, return_type (TObj::*TMethod)(TParams...)&>
			static inline auto make(TObj* obj)noexcept -> const this_type {
		return (this_type (&method_ref_stub<TObj, TMethod>, obj, detail::is_trackable<TObj>::value));
	}

	template<typename TObj, return_type (TObj::*TMethod)(TParams...)const>
			static inline auto make(TObj const* obj)noexcept -> const this_type {
		return (this_type (&method_const_stub<TObj, TMethod>, const_cast<TObj*>(obj), detail::is_trackable<TObj>::value));
	}

	template<typename TObj, return_type (TObj::*TMethod)(TParams...)const&>
			static inline auto make(TObj const* obj)noexcept -> const this_type {
		return (this_type (&method_const_ref_stub<TObj, TMethod>, const_cast<TObj*>(obj), detail::is_trackable<TObj>::value));
	}

	///@brief Invocation operator
	///@details Operator for invoking the
	///specified listener. This operator
	///delegates the specified parameters
	///to the underlying target function.
	///@param An arbitrary number of parameters
	///matching the templated signature of the
	///specific listener class.
	///@return The return type (if any), as
	///specified by the templated signature type
	///of the listener class.
	///@exception Throws if the invoked function throws.
	inline auto operator()(TParams... params)const -> return_type { return ((*m_stub)(m_object, params...)); }

	///@brief Method indicating if the listener is trackable
	///@return boolean indicating whether or not this
	///specific listener is trackable.
	inline auto is_trackable()const noexcept -> bool { return (m_trackable); }

	///@brief Method indicating if the listener has a target
	///@details Method for determining whether or not the
	///specified listener has a target (i.e., is non-null)
	///@return boolean indicating whether or not this
	///specific listener has a valid target
	//inline auto has_target()const noexcept -> bool { return (m_stub); }

	///@brief Method indicating if the listener has a target
	///@details Method for determining whether or not the
	///specified listener has an associated object (i.e. is non-null)
	///@return boolean indicating whether or not this
	///specific listener has a valid target
	inline auto has_object()const noexcept -> bool { return (get_object() != nullptr); }

	///@brief Method for retrieving the associated object
	///@details Method for obtaining the object of the
	///listener (or null, if their is no valid object).
	///@return pointer to the object
	inline auto get_object()const noexcept -> const void* { return (m_object); }

private:
	typedef return_type (*stub_type)(void*, TParams...);

	///@internal
	template <return_type (*TMethod)(TParams...)>
			static inline auto function_stub(void*, TParams... params) -> return_type { return (TMethod)(params...); }

	///@internal
	template<class TObj, return_type (TObj::*TMethod)(TParams...)>
			static inline auto method_stub(void* obj_ptr, TParams... params) -> return_type {
		return (((static_cast<TObj*>(obj_ptr)) ->* TMethod)(params...));
	}

	///@internal
	template<class TObj, return_type (TObj::*TMethod)(TParams...)&>
			static inline auto method_ref_stub(void* obj_ptr, TParams... params) -> return_type {
		return (((static_cast<TObj*>(obj_ptr)) ->* TMethod)(params...));
	}

	///@internal
	template<class TObj, return_type (TObj::*TMethod)(TParams...)const>
			static inline auto method_const_stub(void* obj_ptr, TParams... params) -> return_type {
		return (((static_cast<TObj const*>(obj_ptr)) ->* TMethod)(params...));
	}

	///@internal
	template<class TObj, return_type (TObj::*TMethod)(TParams...)const&>
			static inline auto method_const_ref_stub(void* obj_ptr, TParams... params) -> return_type {
		return (((static_cast<TObj const*>(obj_ptr)) ->* TMethod)(params...));
	}

	///@internal
	listener(stub_type stub_ptr, void* obj_ptr = nullptr, bool trckbl = false)noexcept: m_stub(stub_ptr),
			m_object(obj_ptr), m_trackable(trckbl){}

	stub_type m_stub = nullptr;
	void* m_object = nullptr;
	bool m_trackable = false;
};

}

#endif
