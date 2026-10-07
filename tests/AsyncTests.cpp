#include "Async.hpp"

#include <QSemaphore>
#include <QThread>
#include <QThreadPool>
#include <QtTest/QTest>

#include <atomic>
#include <memory>
#include <optional>
#include <vector>

namespace {

// Lets every worker finish and delivers whatever it queued.
void drain() {
  gxapi::asyncPool().waitForDone();
  QCoreApplication::processEvents();
}

} // namespace

class AsyncTests final : public QObject {
  Q_OBJECT

private Q_SLOTS:
  void doneGetsResultOnReceiverThread();
  void voidWorkStillCallsDone();
  void postsArriveInOrderBeforeDone();
  void receiverDestroyedDuringWorkSkipsDone();
  void receiverDestroyedBeforeDeliverySkipsDone();
  void postsStopOnceReceiverIsDestroyed();
  void retiredPosterDropsLatePosts();
  void longJobsDoNotHoldBackShortOnes();
};

void AsyncTests::doneGetsResultOnReceiverThread() {
  QObject receiver;
  std::atomic<QThread *> workThread{nullptr};
  QThread *doneThread = nullptr;
  std::optional<int> result;

  gxapi::runAsync(
      &receiver,
      [&workThread]() {
        workThread = QThread::currentThread();
        return 42;
      },
      [&](int value) {
        doneThread = QThread::currentThread();
        result = value;
      });
  drain();

  QCOMPARE(result, std::optional<int>(42));
  QCOMPARE(doneThread, receiver.thread());
  QVERIFY(workThread.load() != nullptr);
  QVERIFY(workThread.load() != receiver.thread());
}

void AsyncTests::voidWorkStillCallsDone() {
  QObject receiver;
  bool workRan = false;
  bool doneRan = false;

  gxapi::runAsync(
      &receiver, [&workRan]() { workRan = true; },
      [&]() { doneRan = workRan; });
  drain();

  QVERIFY(doneRan);
}

void AsyncTests::postsArriveInOrderBeforeDone() {
  QObject receiver;
  std::vector<int> order;
  bool postsOnReceiverThread = true;

  gxapi::runAsync(
      &receiver,
      [&](const gxapi::ReceiverPoster &poster) {
        for (int step = 1; step <= 3; ++step) {
          poster.post([&, step]() {
            postsOnReceiverThread =
                postsOnReceiverThread &&
                QThread::currentThread() == receiver.thread();
            order.push_back(step);
          });
        }
        return 4;
      },
      [&order](int last) { order.push_back(last); });
  drain();

  QCOMPARE(order, (std::vector<int>{1, 2, 3, 4}));
  QVERIFY(postsOnReceiverThread);
}

void AsyncTests::receiverDestroyedDuringWorkSkipsDone() {
  auto receiver = std::make_unique<QObject>();
  QSemaphore started;
  QSemaphore finish;
  bool doneRan = false;

  gxapi::runAsync(
      receiver.get(),
      [&]() {
        started.release();
        finish.acquire();
        return 1;
      },
      [&doneRan](int) { doneRan = true; });

  started.acquire();
  receiver.reset();
  finish.release();
  drain();

  QVERIFY(!doneRan);
}

void AsyncTests::receiverDestroyedBeforeDeliverySkipsDone() {
  auto receiver = std::make_unique<QObject>();
  bool doneRan = false;

  gxapi::runAsync(
      receiver.get(), []() { return 1; }, [&doneRan](int) { doneRan = true; });
  // The completion is queued by now but not yet delivered.
  gxapi::asyncPool().waitForDone();
  receiver.reset();
  QCoreApplication::processEvents();

  QVERIFY(!doneRan);
}

void AsyncTests::postsStopOnceReceiverIsDestroyed() {
  auto receiver = std::make_unique<QObject>();
  QSemaphore started;
  std::atomic<bool> rejected{false};
  int delivered = 0;

  gxapi::runAsync(
      receiver.get(),
      [&](const gxapi::ReceiverPoster &poster) {
        started.release();
        // Bounded so that a broken guard fails the test instead of hanging.
        for (int i = 0; i < 100'000; ++i) {
          if (!poster.post([&delivered]() { ++delivered; })) {
            rejected = true;
            return;
          }
          QThread::usleep(10);
        }
      },
      []() {});

  started.acquire();
  QCoreApplication::processEvents();
  const int deliveredBeforeDestroy = delivered;
  receiver.reset();
  drain();

  QVERIFY(rejected.load());
  QCOMPARE(delivered, deliveredBeforeDestroy);
}

void AsyncTests::retiredPosterDropsLatePosts() {
  QObject receiver;
  std::optional<gxapi::ReceiverPoster> kept;
  bool doneRan = false;

  gxapi::runAsync(
      &receiver,
      [&kept](const gxapi::ReceiverPoster &poster) { kept = poster; },
      [&doneRan]() { doneRan = true; });
  drain();
  QVERIFY(doneRan);
  QVERIFY(kept.has_value());

  bool lateRan = false;
  QVERIFY(!kept->post([&lateRan]() { lateRan = true; }));
  QCoreApplication::processEvents();
  QVERIFY(!lateRan);
}

void AsyncTests::longJobsDoNotHoldBackShortOnes() {
  QObject receiver;
  const int longJobs = QThread::idealThreadCount() + 1;
  QSemaphore longStarted;
  QSemaphore releaseLong;
  QSemaphore shortRan;

  for (int i = 0; i < longJobs; ++i) {
    gxapi::runAsync(
        &receiver,
        [&]() {
          longStarted.release();
          releaseLong.acquire();
        },
        []() {});
  }
  gxapi::runAsync(&receiver, [&shortRan]() { shortRan.release(); }, []() {});

  const bool shortRanWhileLongBusy = shortRan.tryAcquire(1, 5000);
  const bool allLongStarted = longStarted.tryAcquire(longJobs, 5000);
  // ~QCoreApplication waits for the global pool, so jobs must stay off it.
  const int globalActive = QThreadPool::globalInstance()->activeThreadCount();
  releaseLong.release(longJobs);
  drain();

  QVERIFY(shortRanWhileLongBusy);
  QVERIFY(allLongStarted);
  QCOMPARE(globalActive, 0);
}

QTEST_GUILESS_MAIN(AsyncTests)
#include "AsyncTests.moc"
