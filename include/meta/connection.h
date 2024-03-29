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
	/*!
	 * \brief The connection_type enum
	 */
	enum connection_type {
		DIRECT,		//!< Direct connection. Slot or callable connected to the signal is invoked synchronously with the invocation of the signal.
		DELAYED		//!< Indirect connection. Call to the slot is stored in a queue and executed when doWork is called on the slot.
	};

	struct connectable_base;

	template <typename ...>
	class signal;

	template <typename ...>
	class slot;

	class connection;

	template <typename ...EmitterArgs, typename ...ReceiverArgs>
	std::shared_ptr<connection> connect(signal<EmitterArgs ...> &,
	                                    slot<ReceiverArgs ...> &,
	                                    connection_type type = connection_type::DIRECT);

	template <typename ...EmitterArgs, typename ...ReceiverArgs>
	std::shared_ptr<connection> connect(signal<EmitterArgs ...> &emitter,
	                                    std::function<void(ReceiverArgs ...)> receiver);
}

namespace meta
{
	struct connectable_base
	{
		std::set<std::shared_ptr<connection>> m_connections;

		friend class connection;

		template <typename ...Args>
		friend class signal;

		template <typename ...Args>
		friend class slot;
	};

	/*!
	 * \brief The type-erased connection "controller".
	 */
	class connection
	{
		connectable_base *m_emitter;
		std::optional<connectable_base *> m_receiver;

		bool m_connected;

		std::weak_ptr<connection> m_me;

        //! Type erased callback
		std::any m_tecb;

		connection()
		    : m_connected(true) {  }

	public:
		connection(connection &) = delete;
		connection(connection &&) = delete;
		connection &operator=(connection &) = delete;
		connection &operator=(connection &&) = delete;

		/*!
		 * \brief The type of the connection, given by ConnectionType
		 */
		connection_type type;

		/*!
		 * \brief Check connection status
		 * \return Returns whether the object is connected
		 */
		bool connected()
		{
			return m_connected;
		}

		/*!
		 * \brief disconnects the connection if it is connected
		 * \return true	- if the connection was connected and is thus now disconnected
		 * false - if the connection was already disconnected
		 */
		bool disconnect()
		{
            if(!m_connected) {
                return false;
            }

            //Acquire a shared pointer to me for map comparison
            auto c = m_me.lock();

            //Remove the connection from the emitter and, if applicable, the sender
            m_emitter->m_connections.erase(c);
            if(m_receiver)
                m_receiver.value()->m_connections.erase(c);

            m_connected = false;
            return true;

		}


		template <typename ...EmitterArgs, typename ...ReceiverArgs>
		friend std::shared_ptr<connection> connect(signal<EmitterArgs ...> &emitter,
		                                           slot<ReceiverArgs ...> &receiver,
		                                           connection_type type);

		template <typename ...EmitterArgs, typename ...ReceiverArgs>
		friend std::shared_ptr<connection> connect(signal<EmitterArgs ...> &emitter,
		                                           std::function<void(ReceiverArgs ...)> receiver);

		template <typename ...Args>
		friend class signal;

		template <typename ...Args>
		friend class slot;
	};


	template <typename ...Args>
	class signal : private connectable_base
	{
	public:
		signal() = default;
		signal(signal &) = delete;
		signal(signal &&) = delete;
		signal &operator=(signal &) = delete;
		signal &operator=(signal &&) = delete;

		~signal()
		{
			for(std::shared_ptr<connection> const &connection : m_connections) {
				if(connection->m_receiver)
					connection->m_receiver.value()->m_connections.erase(connection);
			}

			m_connections.clear();
		}

		/*!
		 * \brief emit the signal with the given arguments
		 * \param ...args : The argument parameter pack
		 */
		void operator()(Args ...args)
		{
			for(std::shared_ptr<connection> const &c : m_connections) {
				std::any_cast<std::function<void(Args ...)>>(c->m_tecb)(args ...);
			}
		}

		template <typename ...EmitterArgs, typename ...ReceiverArgs>
		friend std::shared_ptr<connection> connect(signal<EmitterArgs ...> &emitter,
		                                           slot<ReceiverArgs ...> &receiver,
		                                           connection_type type);

		template <typename ...EmitterArgs, typename ...ReceiverArgs>
		friend std::shared_ptr<connection> connect(signal<EmitterArgs ...> &emitter,
		                                           std::function<void(ReceiverArgs ...)> receiver);
	};

	template <typename ...Args>
	class slot : private connectable_base
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
			for(std::shared_ptr<connection> const &c : m_connections) {
				c->m_emitter->m_connections.erase(c);
			}

			m_connections.clear();
		}

		/*!
		 * \brief Handle the queued up signal emissions
		 */
		void doWork()
		{
			while(!m_queue.empty()) {
				std::apply(m_callable, m_queue.front());
				m_queue.pop();
			}
		}

		/*!
		 * \brief Set the callable for the slot
		 * \param callable A std::function with the correct signature
		 */
		void setCallable(std::function<void(Args ...)> callable)
		{
			m_callable = callable;
		}

		template <typename ...EmitterArgs, typename ...ReceiverArgs>
		friend std::shared_ptr<connection> connect(signal<EmitterArgs ...> &emitter,
		                                           slot<ReceiverArgs ...> &receiver,
		                                           connection_type type);
	};

	/*!
	 * \brief Connects a signal with a slot
	 * \param emitter The emitting signal
	 * \param receiver The receiving sigal
	 * \param type Whether the connection should be initialised as a direct or delayed connection
	 * \return A shared pointer of the connection object.
	 */
	template <typename ...EmitterArgs, typename ...ReceiverArgs>
	std::shared_ptr<connection> connect(signal<EmitterArgs ...> &emitter,
	                                    slot<ReceiverArgs ...> &receiver,
	                                    connection_type type)
	{
		std::shared_ptr<connection> conn(new connection());
		conn->m_me = conn;

		conn->m_emitter = static_cast<connectable_base *>(&emitter);
		conn->m_receiver = static_cast<connectable_base *>(&receiver);
		conn->type = type;

		emitter.m_connections.insert(conn);
		receiver.m_connections.insert(conn);

		auto remapper = [&conn = *conn, &receiver](EmitterArgs ...args){
			switch (conn.type) {
			case connection_type::DIRECT:
				util::apply_drop<ReceiverArgs ...>(receiver.m_callable, args ...);
				break;
			case connection_type::DELAYED:
				auto qe = util::apply_drop<ReceiverArgs ...>(&std::make_tuple<ReceiverArgs ...>, args ...);
				receiver.m_queue.push(qe);
				break;
			}
		};

		conn->m_tecb = std::function<void(EmitterArgs ...)>(remapper);

		return conn;
	}

	/*!
	 * \brief Connects a signal with a std::function defining the receiver args (Ad-hoc connection).
	 *
	 * For callables in another format, use (with F f) std::function(f)
	 * For lambdas, equivalently use std::function([...](...){...})
	 *
	 * \param emitter The emitting signal
	 * \param receiver The receiving std::function
	 * \return A shared pointer to the connection object
	 */
	template <typename ...EmitterArgs, typename ...ReceiverArgs>
	std::shared_ptr<connection> connect(signal<EmitterArgs ...> &emitter,
	                                    std::function<void(ReceiverArgs ...)> receiver)
	{
		std::shared_ptr<connection> conn(new connection());
		emitter.m_connections.insert(conn);

		conn->m_me = conn;

		conn->m_emitter = &emitter;
		conn->m_receiver = std::nullopt;

		conn->type = connection_type::DIRECT;

		auto remapper = [receiver = std::move(receiver)](EmitterArgs ...args){
			util::apply_drop<ReceiverArgs ...>(receiver, args ...);
		};

		conn->m_tecb = std::function<void(EmitterArgs ...)>(remapper);

		return conn;
	}
}
