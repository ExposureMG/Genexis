#pragma once

#include <QMetaObject>
#include <QObject>
#include <QThreadPool>

#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <type_traits>
#include <utility>

namespace gxapi {

// Queues calls from any thread onto a receiver's thread, and never touches the
// receiver once it has been destroyed. A plain QPointer cannot do this: a
// worker could see it non-null and post just as the receiver dies.
//
// Create it on the receiver's thread. Copies share one state.
class ReceiverPoster {
public:
  explicit ReceiverPoster(QObject *receiver)
      : m_state(std::make_shared<State>()) {
    m_state->receiver = receiver;
    // ~QObject emits destroyed() before it discards the object's pending
    // events, so a call posted before the reset below is discarded with them.
    m_state->destroyedConnection =
        QObject::connect(receiver, &QObject::destroyed, [state = m_state]() {
          const std::lock_guard lock(state->mutex);
          state->receiver = nullptr;
        });
  }

  // Runs fn on the receiver's thread, unless the receiver is gone or this
  // poster has been retired. Returns whether fn was queued.
  template <typename Fn> bool post(Fn &&fn) const {
    const std::lock_guard lock(m_state->mutex);
    if (!m_state->receiver) {
      return false;
    }
    return QMetaObject::invokeMethod(m_state->receiver, std::forward<Fn>(fn),
                                     Qt::QueuedConnection);
  }

  // Stops all further posts and releases the receiver connection. Call it on
  // the receiver's thread.
  void retire() const {
    {
      const std::lock_guard lock(m_state->mutex);
      m_state->receiver = nullptr;
    }
    QObject::disconnect(m_state->destroyedConnection);
  }

private:
  struct State {
    std::mutex mutex;
    QObject *receiver{nullptr};
    QMetaObject::Connection destroyedConnection;
  };

  std::shared_ptr<State> m_state;
};

// The pool runAsync uses. It is not QThreadPool::globalInstance(), because
// ~QCoreApplication waits for that one: closing the window during a long flash
// or build would leave a windowless process until the call returned, or
// forever if it hung. It is never destroyed, for the same reason, and has no
// thread limit, so minutes-long flash or build jobs cannot hold back short
// ones such as loading a NAND file.
inline QThreadPool &asyncPool() {
  static QThreadPool *const pool = [] {
    auto *created = new QThreadPool;
    created->setMaxThreadCount(std::numeric_limits<int>::max());
    return created;
  }();
  return *pool;
}

// Runs work() on asyncPool(), then done(result) on the receiver's thread. If
// the receiver is destroyed first, done is dropped and the receiver is never
// touched. work may instead take a const ReceiverPoster & to queue
// intermediate calls such as progress updates, which arrive before done.
//
// Call it on the receiver's thread.
template <typename Work, typename Done>
void runAsync(QObject *receiver, Work work, Done done) {
  const ReceiverPoster poster(receiver);
  asyncPool().start(
      [poster, work = std::move(work), done = std::move(done)]() mutable {
        const auto finish = [&poster](auto callback) {
          poster.post([poster, callback = std::move(callback)]() mutable {
            poster.retire();
            callback();
          });
        };
        const auto runWork = [&]() -> decltype(auto) {
          if constexpr (std::is_invocable_v<Work &, const ReceiverPoster &>) {
            return std::invoke(work, std::as_const(poster));
          } else {
            return std::invoke(work);
          }
        };

        using Result = decltype(runWork());
        if constexpr (std::is_void_v<Result>) {
          runWork();
          finish([done = std::move(done)]() mutable { std::invoke(done); });
        } else {
          finish([done = std::move(done), result = runWork()]() mutable {
            std::invoke(done, std::move(result));
          });
        }
      });
}

} // namespace gxapi
