#pragma once
// Этап 2: лок, о котором знает планировщик. Поток, ждущий занятый лок, не
// попадает в enabled; обычный std::mutex здесь нельзя — поток уснёт с ходом в
// руках и всё зависнет. Лог: "lock <имя>", "unlock <имя>". Планировщик должен
// знать все локи, чтобы печатать held={a:A, b:B}.
#include <stdexcept>
#include <string>

#include "scheduler.hpp"

namespace sched {

class sched_mutex {
public:
  // Создаётся в setup() тестовой программы; зарегистрируйте лок в планировщике
  // s.
  sched_mutex(Scheduler &s, std::string name)
      : scheduler_(s), name(std::move(name)), owner_('\0') {
    s.register_mutex(this);
  }

  sched_mutex(const sched_mutex &) = delete;

  // Пауза; если лок занят — ждать, не участвуя в выборе; затем стать
  // владельцем.
  void lock() {
    auto &thread_state = scheduler_.get_current_thread_state();

    scheduler_.pause();

    while (owner_ != '\0') {
      thread_state.wait_mutex = this;
      scheduler_.yield_to_sched();
      thread_state.wait_mutex = nullptr;
    }

    owner_ = thread_state.id;
    thread_state.touched = true;
    scheduler_.record("lock " + name);
  }

  // Пауза; освободить лок, ждавшие потоки снова могут быть выбраны.
  void unlock() {
    scheduler_.pause();
    owner_ = '\0';
    scheduler_.get_current_thread_state().touched = true;
    scheduler_.record("unlock " + name);
  }

  const std::string name;

private:
  Scheduler &scheduler_;
  char owner_;
};

// RAII: lock сейчас, unlock на закрывающей скобке. Выдан готовым.
class sched_guard {
public:
  explicit sched_guard(sched_mutex &m) : m_(m) { m_.lock(); }
  ~sched_guard() noexcept(false) { m_.unlock(); }
  sched_guard(const sched_guard &) = delete;

private:
  sched_mutex &m_;
};

} // namespace sched
