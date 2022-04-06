#pragma once

#include <concepts>
#include <variant>

#include "connection.h"

namespace meta
{
	template <typename T>
	class object;

	template <std::copy_constructible T>
	class property;
}

namespace meta {
	using std::variant;

	template <typename T>
	class object
	{
	public:
		object()
		{

		}

	protected:
		virtual void properties() = 0;
	};

	template <std::copy_constructible T>
	class property
	{
		T m_value;

	public:
		property(T const& value = T()) : m_value(value)
		{
			set = [this](T value)
			{
				m_value = value;
				changed(value);
			};
		}

		signal<T> changed;
		slot<T> set;

		operator T() const
		{
			return m_value;
		}

		T operator=(T const& value)
		{
			m_value = value;
			changed(value);

			return *this;
		}
	};
}
