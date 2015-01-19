#ifndef NL_NOTIFIER_HPP
#define NL_NOTIFIER_HPP

#include <algorithm>
#include <list>
#include "listener.hpp"
#include "trackable.hpp"

namespace nl{
template<typename TFunc> class notifier;

///@class notifier
///@brief notifier class for connecting/broadcasting listeners.
///@details This class is responsible for connecting, broadcasting,
///and disconnecting listeners, through the connect, notify,
///and disconnect/connection interfaces, respectfully.
///@ingroup notifier
template<typename TReturn, typename... TParams> class notifier<TReturn (TParams...)> {
	typedef listener<TReturn (TParams...)> lstnr_type;
	typedef std::list<lstnr_type> cntnr_type;

public:
	typedef notifier<TReturn (TParams...)> this_type;
	typedef TReturn return_type;

	///@class connection
	///@brief connection class for disconnecting listeners
	///@details this class is used track and disconnect
	///listeners, through the provided methods.
	///@warning the validity of connected connection
	///objects is only guaranteed while it's associated
	///notifier class is valid; otherwise, their state
	///is undefined.
	class connection{
	public:
		friend class notifier<TReturn (TParams...)>;

		connection(const connection&) = delete;
		connection(connection&&)noexcept = default;

		auto operator=(const connection&) -> connection& = delete;
		auto operator=(connection&&)noexcept -> connection& = default;

		///@brief method for disconnecting listeners
		///@details method for disconnecting listeners
		///from their associated notifier. Once disconnected,
		///this method may be called arbitrarily, albeit
		///redundantly, with no adverse side effects, for as
		///long as the associated notifier object persists.
		///@remarks O(1) in complexity
		auto disconnect() -> void {
			if(m_notifier){
				m_notifier -> disconnect(m_listener);
				m_notifier = nullptr;
			}
		}

	private:
		connection(this_type* notifier, typename cntnr_type::iterator lstnr_iter)noexcept: m_notifier(notifier),
				m_listener(lstnr_iter){}

		this_type* m_notifier = nullptr;
		typename cntnr_type::iterator m_listener;
	};

	notifier() = default;
	notifier(const this_type& notifier) = default;
	notifier(this_type&& notifier) = default;

	auto operator=(const this_type& notifier) -> this_type& = default;
	auto operator=(this_type&& notifier) -> this_type& = default;

	~notifier()noexcept{
		//notify any trackable listeners to our destruction
		for(const auto& lstnr: m_listeners){
			if(lstnr.is_trackable()){
				static_cast<const trackable*>(lstnr.get_object()) -> untrack(this);
			}
		}
	}

	///@brief invokes the notifier
	///@details invokes a notifier object, causing it
	///to alert all of its associated listeners.
	///@param[in] an arbitrary number of parameters
	///matching the templated function type of the notifier.
	///@exception throws if any of the invoked listeners throws.
	///@remarks O(n) in invocation.
	auto operator()(TParams... params)const -> void {
		for(const auto& lstnr: m_listeners){
			lstnr(params...);
		}
	}

	///@brief Method for connecting a listener to the notifier
	///@details Method for connecting a specific listener
	///to the notifier object. The specified listener is
	///copied/moved into the notifier, and is invoked
	///whenever the notifier itself is invoked, unless
	///manually disconnected by the associated connection
	///object (or automatically, if derived from the
	///trackable class).
	///@param[in] lstnr - The listener to be added
	///to the notifier.
	///@return A connection object that is directly
	///associated with the connected listener. This
	///object is used to disconnect the listener
	///when no longer wanted.
	///@exception May throw
	template<typename TLstnr> auto connect(TLstnr&& lstnr) -> connection {
		if(lstnr.is_trackable()){
			static_cast<const trackable*>(lstnr.get_object()) -> track(this, &disconnect_trackable);
		}

		m_listeners.push_front(std::forward<TLstnr>(lstnr));

		return (connection(this, m_listeners.begin()));
	}

	///@brief Disconnects an object from the notifier
	///@details Disconnects a specific object from the
	///notifier. Any and all listeners associated with
	///the object will be disconnected.
	///@param[in] An object to be disconnected.
	///@exception May throw.
	///@remarks Note that this method acts on INSTANCES
	///of a specific object, rather a general class.
	template<typename TObj> inline auto disconnect(const TObj* obj) -> void {
		m_listeners.erase(std::remove_if(m_listeners.begin(), m_listeners.end(),
				[=](const lstnr_type& lstnr) -> bool { return (lstnr.get_object() == obj); }));
	}

	///@brief disconnects all listeners
	///@details disconnects all (if any)
	///associated listeners from the notifier.
	inline auto disconnect_all()noexcept -> void { m_listeners.clear(); }

	///@see operator()
	inline auto notify(TParams... params)const -> void { (*this)(params...); }

private:
	///@internal thunk for disconnecting trackable objects.
	static inline auto disconnect_trackable(void* notifier, const trackable* obj) -> void
			{ (static_cast<this_type*>(notifier)) -> disconnect(obj); }

	///@internal disconnects the specified listener.
	inline auto disconnect(typename cntnr_type::iterator lstnr_iter) -> void { m_listeners.erase(lstnr_iter); }

	cntnr_type m_listeners;
};

}

#endif
