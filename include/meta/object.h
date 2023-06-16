#pragma once

#include <concepts>
#include <variant>
#include <list>
#include <map>

#include "connection.h"

namespace meta
{
	template <typename T>
	concept Objectable = requires (T t) {
	    t.properties();
    };

	template <typename>
	class object;

	template <std::copy_constructible>
	class property;

}

namespace meta {
	using std::variant;

	template <typename Class>
	//reqires Objectable<Class>
	class object
	{
		std::list<std::function<void()>> m_propertyUpdater;
		std::map<std::string, std::any> m_namedProperties;

	public:
		object(Class *implementer)
		{
			implementer->properties();
		}

		template <typename T>
		void addProperty(property<T> &p, std::optional<std::string> name = std::nullopt)
		{
			m_propertyUpdater.push_back([&p](){
				p.set.doWork();
			});

			if(name) {
				m_namedProperties[name.value()] = &p;
			}
		}

		void update()
		//requires Objectable<Class>
		{
			for(auto &x : m_propertyUpdater) {
				x();
			}
		}

		std::any getPropertyByName(std::string name)
		{
			if(m_namedProperties.contains(name)) {
				return m_namedProperties[name];
			} return std::any();
		}
	};

	template <std::copy_constructible T>
	class property
	{
		T m_value;

	public:
		property(T const& value = T()) : m_value(value)
		{
			set.setCallable([this](T value)
			{
				m_value = value;
				changed(value);
			});
		}

		signal<T> changed;
		slot<T> set;

		operator T() const
		{
			return m_value;
		}

		template <typename U>
		U operator=(U const& value)
		{
			m_value = value;
			changed(m_value);

			return *this;
		}
	};
}
