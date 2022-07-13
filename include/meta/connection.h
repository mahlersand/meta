#pragma once

#include <memory>
#include <atomic>
#include <functional>
#include <set>
#include <concepts>
#include <queue>
#include <any>
#include <optional>

#include "util.h"

namespace meta
{
	enum connectionType {
		DIRECT,
		DELAYED
	};

	struct connectable_base;

	template <typename ...>
	class signal;

	template <typename ...>
	class slot;

	class connection;

	template <typename ...>
	class signal_connection;

	template <typename ...>
	class slot_connection;

	template <typename ...EmitterArgs, typename ...ReceiverArgs>
	std::shared_ptr<connection> connect(signal<EmitterArgs ...> &,
	                                    slot<ReceiverArgs ...> &,
	                                    connectionType type = connectionType::DIRECT);
}

namespace meta
{
	using std::size_t;
	using std::shared_ptr;
	using std::set;
	using std::function;

	struct connectable_base
	{
	protected:
		set<shared_ptr<connection>> m_connections;

		friend struct connection;

		template <typename ...Args>
		friend struct signal;

		template <typename ...Args>
		friend struct slot;
	};

	class connection
	{
		using type_t = connectionType;

		connectable_base *m_emitter;
		std::optional<connectable_base *> m_receiver;

		bool m_connected;

		std::weak_ptr<connection> m_me;

		std::any m_tecb;

		connection()
		    : m_connected(true) {  }

	public:
		connection(connection &) = delete;
		connection(connection &&) = delete;
		connection &operator=(connection &) = delete;
		connection &operator=(connection &&) = delete;

		type_t type;

		bool connected()
		{
			return m_connected;
		}

		bool disconnect()
		{
			if(m_connected) {
				auto c = m_me.lock();
				m_emitter->m_connections.erase(c);
				if(m_receiver)
					m_receiver.value()->m_connections.erase(c);

				m_connected = false;
				return true;
			} else return false;
		}

		template <typename ...EmitterArgs, typename ...ReceiverArgs>
		friend shared_ptr<connection> connect(signal<EmitterArgs ...> &emitter,
		                                      slot<ReceiverArgs ...> &receiver,
		                                      type_t type);

		template <typename ...EmitterArgs, typename ...ReceiverArgs>
		friend shared_ptr<connection> connect(signal<EmitterArgs ...> &emitter,
		                                      std::function<void(ReceiverArgs ...)> receiver);

		template <typename ...Args>
		friend struct signal;

		template <typename ...Args>
		friend struct slot;
	};


	template <typename ...Args>
	class signal : public connectable_base
	{
	public:
		signal() = default;
		signal(signal &) = delete;
		signal(signal &&) = delete;
		signal &operator=(signal &) = delete;
		signal &operator=(signal &&) = delete;

		~signal()
		{
			for(shared_ptr<connection> const &c : m_connections) {
				if(c->m_receiver)
					c->m_receiver.value()->m_connections.erase(c);
			}

			m_connections.clear();
		}

		void operator()(Args ...args)
		{
			for(shared_ptr<connection> const &c : m_connections) {
				std::any_cast<function<void(Args ...)>>(c->m_tecb)(args ...);
			}
		}

		template <typename ...EmitterArgs, typename ...ReceiverArgs>
		friend shared_ptr<connection> connect(signal<EmitterArgs ...> &emitter,
		                                      slot<ReceiverArgs ...> &receiver,
		                                      connection::type_t type);

		template <typename ...EmitterArgs, typename ...ReceiverArgs>
		friend shared_ptr<connection> connect(signal<EmitterArgs ...> &emitter,
		                               std::function<void(ReceiverArgs ...)> receiver);
	};

	template <typename ...Args>
	class slot : public connectable_base
	{
		std::function<void(Args ...)> m_callable;
		std::queue<std::tuple<Args ...>> m_queue;

	public:
		slot() = default;
		slot(slot &) = delete;
		slot(slot &&) = delete;
		slot &operator=(slot &) = delete;
		slot &operator=(slot &&) = delete;
		slot &operator=(std::function<void(Args ...)> f)
		{
			setCallable(f);
		}

		~slot()
		{
			for(shared_ptr<connection> const &c : m_connections) {
				c->m_emitter->m_connections.erase(c);
			}

			m_connections.clear();
		}

		void doWork()
		{
			while(!m_queue.empty()) {
				std::apply(m_callable, m_queue.front());
				m_queue.pop();
			}
		}

		void setCallable(std::function<void(Args ...)> callable)
		{
			m_callable = callable;
		}

		template <typename ...EmitterArgs, typename ...ReceiverArgs>
		friend shared_ptr<connection> connect(signal<EmitterArgs ...> &emitter,
		                                      slot<ReceiverArgs ...> &receiver,
		                                      connection::type_t type);

		template <typename ...EmitterArgs, typename ...ReceiverArgs>
		friend shared_ptr<connection> connect(signal<EmitterArgs ...> &emitter,
		                               std::function<void(ReceiverArgs ...)> receiver);
	};

	template <typename ...EmitterArgs, typename ...ReceiverArgs>
	shared_ptr<connection> connect(signal<EmitterArgs ...> &emitter,
	                               slot<ReceiverArgs ...> &receiver,
	                               connectionType type)
	{
		std::shared_ptr<connection> conn(new connection());
		conn->m_me = conn;

		conn->m_emitter = &emitter;
		conn->m_receiver = &receiver;
		conn->type = type;

		emitter.m_connections.insert(conn);
		receiver.m_connections.insert(conn);

		connection &conn_ref = *conn;

		auto remapper = [&conn_ref](EmitterArgs ...args){
			switch (conn_ref.type) {
			case DIRECT:
				util::apply_drop<ReceiverArgs ...>(static_cast<slot<ReceiverArgs ...> *>(conn_ref.m_receiver.value())->m_callable, args ...);
				break;
			case DELAYED:
				auto qe = util::apply_drop<ReceiverArgs ...>(&std::make_tuple<ReceiverArgs ...>, args ...);
				static_cast<slot<ReceiverArgs ...> *>(conn_ref.m_receiver.value())->m_queue.push(qe);
				break;
			}
		};

		conn->m_tecb = std::function<void(EmitterArgs ...)>(remapper);

		return conn;
	}

	template <typename ...EmitterArgs, typename ...ReceiverArgs>
	shared_ptr<connection> connect(signal<EmitterArgs ...> &emitter,
	                               std::function<void(ReceiverArgs ...)> receiver)
	{
		std::shared_ptr<connection> conn(new connection());
		conn->m_me = conn;

		conn->m_emitter = &emitter;
		conn->m_receiver = std::nullopt;

		emitter.m_connections.insert(conn);

		connection &conn_ref = *conn;

		auto remapper = [receiver](EmitterArgs ...args){
			util::apply_drop<ReceiverArgs ...>(receiver, args ...);
		};

		conn->m_tecb = std::function<void(EmitterArgs ...)>(remapper);

		return conn;
	}
}
