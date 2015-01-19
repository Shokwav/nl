#ifndef NL_TRACKABLE_HPP
#define NL_TRACKABLE_HPP

#include <algorithm>
#include <forward_list>
#include <type_traits>

namespace nl{
///@class trackable
///@brief Base class for automatically disconnectable listeners.
///@details This class is used when trackable listeners are
///desired. Simply (publicly) inherit from this class to
///enable derived objects to be automatically disconnected.
///@ingroup trackable
class trackable{
public:
	typedef void (*track_func)(void* notifier, const trackable* obj);

	trackable() = default;
	inline trackable(const trackable& trackable) = default;
	inline trackable(trackable&& trackable) = default;

	auto operator=(const trackable& notifier) -> trackable& = default;
	auto operator=(trackable&& notifier) -> trackable& = default;

	inline ~trackable(){
		for(auto& cllbck: m_callbacks){
			cllbck();
		}
	}

	///@brief Method for tracking notifiers
	///@details This methods adds the specified notifier
	///to the trackable object. These notifiers will
	///automatically be alerted when the trackable
	///object is destroyed.
	///@param[in] notifier - The notifier to track
	///@param[in] func - The callback to execute when
	///the trackable object is destroyed
	///@exception May throw
	template<typename TNotifier> inline auto track(TNotifier* notifier, track_func func)const -> void
			{ m_callbacks.push_front({func, static_cast<void*>(notifier), this}); }

	///@brief Method to stop tracking notifiers
	///@details This method removes the specified notifier
	///from the trackable object.
	///@param[in] notifier - The notifier to stop tracking
	template<typename TNotifier> inline auto untrack(const TNotifier* notifier)const noexcept -> void
			{ m_callbacks.remove_if([=](const callback& cllbck) -> bool { return (cllbck.notifier == notifier); }); }

private:
	///@internal
	struct callback{
		track_func function;
		void* notifier;
		const trackable* object;

		inline auto operator()() -> void { (*function)(notifier, object); }
	};

	mutable std::forward_list<callback> m_callbacks;
};

namespace detail{
///@internal
template<typename TObj> struct is_trackable{
	static constexpr const bool value = std::is_base_of<trackable, TObj>::value;
};

}

}

#endif
