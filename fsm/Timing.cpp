#include "Timing.h"

#include <absl/strings/str_format.h>
#include <absl/synchronization/mutex.h>

#include <optional>

namespace Timing {
std::optional<types::Transition> Timer::Start() const {

  if (configs.empty())
    return std::nullopt;

  rxcpp::subjects::subject<bool> stop_trigger;
  std::atomic<bool> stopped{false};
  absl::Mutex mutex;
  absl::CondVar cond;
  bool triggered ABSL_GUARDED_BY(mutex) = false;
  std::optional<TimerConfig> result ABSL_GUARDED_BY(mutex);

  // All the callbacks could be made into their own function if need be
  rxcpp::rxs::iterate(configs, rxcpp::observe_on_new_thread())
      .flat_map([&stop_trigger](const TimerConfig& cfg) {
        //! this causes every timer to run on it's own thread
        //! if that is an issue change rxcpp::observe_on_new_thread() -> rxcpp::serialize_new_thread()
        return rxcpp::sources::timer(cfg.duration,
                                     rxcpp::serialize_new_thread())
            .map([cfg](long) { return cfg; })
            .take_until(stop_trigger.get_observable());
      })
      .subscribe(
          [&](const TimerConfig& cfg) {
            std::cerr << absl::StrFormat("Timer %s finished naturally!\n",
                                         cfg.name);

            if (bool am_i_first = !stopped.exchange(true)) {
              std::cerr << ">>> Signaling other timers to stop... <<<"
                        << std::endl;
              stop_trigger.get_subscriber().on_next(
                  true);  // Emits a value for take_until
              stop_trigger.get_subscriber()
                  .on_completed();  // Complete the trigger

              // store the result
              {
                absl::MutexLock lock(&mutex);
                result = cfg;
              }
            }
            // unblock main thread
            {
              absl::MutexLock lock(&mutex);
              triggered = true;
            }
            cond.Signal();
          },
          // on error
          [&](const std::exception_ptr& e) {
            try {
              std::rethrow_exception(e);
            } catch (const std::exception& ex) {
              std::cerr << absl::StrFormat("[ERROR]: %s", ex.what())
                        << std::endl;
            }
            {
              absl::MutexLock lock(&mutex);
              if (!triggered) triggered = true;

            }
            cond.Signal();
          },
          // on_completed
          [&]() {
            std::cerr << "--- All timer sequences have completed. ---"
                      << std::endl;
            {
              absl::MutexLock lock(&mutex);
              if (!triggered) triggered = true;
            }
            cond.Signal();
          });
  // block main thread
  {
    absl::MutexLock mtx(&mutex);
    while (!triggered) cond.Wait(&mutex);
  }

  {
    absl::MutexLock lock(&mutex);
    std::cerr << "Len of queue: " << results.size() << std::endl;
    std::cerr << "Trying to read the result" << std::endl;
    if (result.has_value()) {
      return result.value().transition;
    }
  }
  return std::nullopt;
}
}  // namespace Timing