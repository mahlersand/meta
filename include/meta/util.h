#pragma once

#include <tuple>
#include <utility>
#include <memory>

//Documentation

/*!
 *	\namespace meta
 *	The meta namespace contains all the functionality of the meta library
 */


namespace meta {
	namespace util {
		using std::tuple;
		using std::size_t;
		using std::get;
		using std::forward;
		using std::index_sequence;
		using std::index_sequence_for;
		using std::shared_ptr;

		template <typename F, typename ...Args>
		concept functorable = std::invocable<F, Args ...> && std::copy_constructible<F>;

		template <typename F, size_t ...Indices, typename ...SourceTypes>
		constexpr decltype(auto) apply_drop_impl(F &&f,
		                                         index_sequence<Indices ...>,
		                                         SourceTypes ...sourceValues)
		{
			return f(get<Indices>(std::make_tuple(sourceValues ...)) ...);
		}

		template <typename ...TargetTypes, typename F, typename ...SourceTypes>
		constexpr decltype(auto) apply_drop(F &&f,
		                                    SourceTypes ...sourceValues)
		{
			return apply_drop_impl(forward<F>(f),
			                       index_sequence_for<TargetTypes ...>{},
			                       sourceValues ...);
		}
	}
}
