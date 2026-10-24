#pragma once
// Этап 2: лок, о котором знает планировщик. Поток, ждущий занятый лок, не
// попадает в enabled; обычный std::mutex здесь нельзя — поток уснёт с ходом в
// руках и всё зависнет. Лог: "lock <имя>", "unlock <имя>". Планировщик должен
// знать все локи, чтобы печатать held={a:A, b:B}.
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

    //
    // Нельзя просто (взять и) написать паузу + цикл ожидания разблокировки,
    // так как в таком случае поток может на паузе передать управление
    // планировщику без выставленного wait_mutex, из-за чего добавит его в enabled_,
    // хотя не должен был бы.
    //
    if (thread_state.touched || owner_ != '\0') {
      do {
        thread_state.touched = false;
        thread_state.wait_mutex = this;
        scheduler_.yield_to_sched();
        thread_state.wait_mutex = nullptr;
      } while (owner_ != '\0');
    }

    owner_ = thread_state.id;
    thread_state.touched = true;
    scheduler_.record("lock " + name);
  }

  // Пауза; освободить лок, ждавшие потоки снова могут быть выбраны.
  void unlock() {
    if (scheduler_.is_aborted()) {
      return;
    }

    scheduler_.pause();

    owner_ = '\0';
    scheduler_.get_current_thread_state().touched = true;
    scheduler_.record("unlock " + name);
  }

  const std::string name;

  char getOwner() { return owner_; }

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
