#pragma once
// Этап 1: планировщик, который ведёт настоящие потоки строго по одному
// (передача хода через семафоры). Этап 2: обнаружение deadlock в главном цикле
// и корректное завершение застрявших потоков. Внутреннее устройство (поля,
// семафоры, вспомогательные функции для shared и sched_mutex) придумываете
// сами.
#include <functional>
#include <memory>
#include <semaphore>
#include <string>
#include <thread>
#include <vector>

#include "strategy.hpp"

namespace sched {

class Scheduler {
public:
  static Scheduler *get_scheduler();
  static char get_thread_id();

  Scheduler() = default;
  ~Scheduler();
  // Зарегистрировать поток. Потоки получают буквы A, B, C… в порядке вызова
  // spawn.
  void spawn(std::function<void()> body);
  // Запустить потоки и вести их по одному: на каждой паузе спросить
  // strategy.pick(enabled, last).
  void run(Strategy &strategy);
  // Трасса прогона: буквы потоков, выбранных во всех решениях подряд, например
  // "AAAABB".
  std::string trace_string() const;

  // Если true, каждая операция с общими данными добавляет строку в log.
  // Выставляет runner.
  bool logging = false;
  // Лог прогона: строки "<поток>  <операция>", например "A  lock mu", "A
  // balance -> 100".
  std::vector<std::string> log;
  // Пустая строка, если deadlock не было; иначе "held={a:A, b:B}" (этап 2).
  std::string deadlock;

  void record(const std::string &str);
  void pause();
  void yield_to_sched();

  struct ThreadState {
    char id;

    std::function<void()> body;
    std::thread thread;

    std::binary_semaphore semaphore{0};

    bool finished = false;
    //
    // "Без пауз подряд.
    // Если с прошлой паузы поток не трогал ничего общего,
    // следующая операция выполняется без паузы."
    //
    // Изначально true, чтобы выполнить потоки со старта до первой паузы.
    //
    bool touched = true;
    std::function<bool()> cond = nullptr;
  };

  ThreadState &get_current_thread_state() {
    return *threads_[get_thread_id() - 'A'];
  };

private:
  std::string trace_;
  std::vector<std::unique_ptr<ThreadState>> threads_;
  std::binary_semaphore sched_semaphore_{0};
};

// window() из тестовых программ: пауза перед следующей операцией с общими
// данными обязательна.
void point();
// Ждать, пока cond не станет true. До этого поток не попадает в enabled.
void await(std::function<bool()> cond);

struct TlsStorage {
  Scheduler *scheduler = nullptr;
  char thread_id = '\0';
};

} // namespace sched
