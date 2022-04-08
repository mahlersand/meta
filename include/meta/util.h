#pragma once

#include <tuple>
#include <utility>

namespace meta {
	namespace detail {
		//using std::tuple;
		using std::size_t;
		using std::get;
		using std::forward;
		using std::index_sequence;
		using std::index_sequence_for;

		template <typename F, class Tuple, size_t ...Indices>
		constexpr decltype(auto) apply_drop_impl(F &&f, Tuple t, index_sequence<Indices ...>)
		{
			return f(get<Indices>(t)...);
		}

		template <typename ...TargetTypes, typename F, typename Tuple>
		constexpr decltype(auto) apply_drop(F &&f, Tuple t)
		{
			return apply_drop_impl(forward<F>(f), t, index_sequence_for<TargetTypes ...>{});
		}
	}
}
