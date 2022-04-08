#pragma once

#include <memory>
#include <atomic>
#include <functional>
#include <map>
#include <concepts>

#include "util.h"

namespace meta
{
	template <typename F, typename ...Args>
	concept functorable = std::invocable<F, Args ...> && std::copy_constructible<F>;

	template <typename ...>
	class signal;

	template <typename ...>
	class slot;

	template <typename ...>
	class connection;

	template <typename ...Args>
	connection<Args ...> connect(signal<Args ...> &emitter, slot<Args ...> &receiver);
}

namespace meta
{
	using std::size_t;

	template <typename ...Args>
	class signal
	{
		std::map<std::uint_fast64_t, connection<Args ...>> m_connections;

	public:
		signal() = default;
		signal(signal &) = delete;
		signal(signal &&) = delete;
		signal &operator=(signal &) = delete;
		signal &operator=(signal &&) = delete;
		~signal()
		{
			for(auto& [id, connection] : m_connections) {
				connection.receiver.m_connections.erase(id);
			}
		}

		void operator()(Args ...args)
		{
			for(auto& [id, connection] : m_connections) {
				connection.receiver.m_callable(args ...);
			}
		}

		size_t numConnections()
		{
			return m_connections.size();
		}

		friend connection<Args ...> connect<Args ...>(signal<Args ...> &emitter, slot<Args ...> &receiver);
		friend class connection<Args ...>;
		friend class slot<Args ...>;
	};

	template <typename ...Args>
	class slot
	{
		std::function<void(Args ...)> m_callable;
		std::map<std::uint_fast64_t, connection<Args ...>> m_connections;

	public:
		slot() = default;
		slot(slot &) = delete;
		slot(slot &&) = delete;
		slot &operator=(slot &) = delete;
		slot &operator=(slot &&) = delete;
		~slot()
		{
			for(auto& [id, connection] : m_connections) {
				connection.emitter.m_connections.erase(id);
			}
		}

		template <typename F>
		requires functorable<F, Args...>
		void operator=(F f)
		{
			m_callable = f;
		}

		void operator()(Args ...args)
		{
			m_callable(args ...);
		}

		size_t numConnections()
		{
			return m_connections.size();
		}

		friend connection<Args ...> connect<Args ...>(signal<Args ...> &emitter, slot<Args ...> &receiver);
		friend class connection<Args ...>;
		friend class signal<Args ...>;
	};

	template <typename ...Args>
	class connection
	{
		inline static std::atomic_uint_fast64_t s_counter = 0;
		unsigned long long m_id;
		signal<Args ...> &emitter;
		slot<Args ...> &receiver;

		connection(signal<Args ...> &emitter,
		           slot<Args ...> &receiver)
		    : m_id(++s_counter),
		      emitter(emitter),
		      receiver(receiver)
		{

		}

	public:
		void disconnect()
		{
			emitter.m_connections.erase(m_id);
			receiver.m_connections.erase(m_id);
		}

		friend connection<Args ...> connect<Args ...>(signal<Args ...> &emitter, slot<Args ...> &receiver);
		friend class signal<Args ...>;
		friend class slot<Args ...>;
	};

	template <typename ...Args>
	connection<Args ...> connect(signal<Args ...> &emitter, slot<Args ...> &receiver)
	{
		connection<Args ...> c(emitter, receiver);

		emitter.m_connections.insert(std::make_pair(c.m_id, c));
		receiver.m_connections.insert(std::make_pair(c.m_id, c));

		return c;
	}
}
