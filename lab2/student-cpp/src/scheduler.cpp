// Этап 1–2: реализация Scheduler, point() и await().
// Сейчас все методы бросают исключение «TODO»: заполните их по заданию этапов 1
// и 2. Деструктор должен дождаться (join) всех запущенных потоков, в том числе
// после deadlock.
#include "scheduler.hpp"

#include <sstream>

namespace sched {

static thread_local TlsStorage tls_storage;

Scheduler *Scheduler::get_scheduler() { return tls_storage.scheduler; }
char Scheduler::get_thread_id() { return tls_storage.thread_id; }

Scheduler::~Scheduler() {
  for (auto &thread_state : threads_) {
    if (thread_state->thread.joinable()) {
      thread_state->thread.join();
    }
  }
}

void Scheduler::spawn(std::function<void()> body) {
  auto thread_state = std::make_unique<ThreadState>();

  thread_state->id = 'A' + threads_.size();
  thread_state->body = std::move(body);

  threads_.emplace_back(std::move(thread_state));
}

void Scheduler::run(Strategy &strategy) {
  size_t n = threads_.size();
  if (n == 0) {
    return;
  }

  for (size_t i = 0; i < n; ++i) {
    threads_[i]->thread = std::thread([this, i]() {
      tls_storage.scheduler = this;
      tls_storage.thread_id = 'A' + i;

      //
      // На shared поток добавит 1 в семафор планировщика, разбудит его,
      // а сам заснет на своем семафоре, пока его не разбудит планировщик.
      //
      threads_[i]->body();

      //
      // Поток завершается, мы должны вернутьс в планировщик.
      //
      threads_[i]->finished = true;
      sched_semaphore_.release();
    });

    sched_semaphore_.acquire();
  }

  //
  // Теперь все потоки на паузе спят на своих семафорах.
  //

  int last = -1;
  while (true) {
    std::vector<int> enabled;
    int alive = 0;

    for (int i = 0; i < n; ++i) {
      ThreadState &thread_state = *threads_[i];
      if (thread_state.finished) {
        continue;
      }

      alive++;

      if (thread_state.cond != nullptr && !thread_state.cond()) {
        continue;
      }

      enabled.push_back(i);
    }

    if (alive == 0) {
      break;
    }

    int picked = strategy.pick(enabled, last);
    last = picked;
    trace_.push_back('A' + picked);

    //
    // Отдаем жетон семафору выбранного потока, чтобы он продолжил выполнение
    // с паузы, а планировщик блокируем на своем семафоре (уже ноль, так как мы
    // сами же забрали жетон, когда вошли в планировщик), ожидая, что на
    // следующей паузе (или в конце выполнения) выбранный поток вернет жетон
    // планировщику.
    //
    threads_[picked]->semaphore.release();
    sched_semaphore_.acquire();
  }

  for (auto &t : threads_) {
    if (t->thread.joinable()) {
      t->thread.join();
    }
  }
}

std::string Scheduler::trace_string() const { return trace_; }

void point() {
  Scheduler *scheduler = Scheduler::get_scheduler();
  scheduler->get_current_thread_state().touched = true;
  scheduler->record("window");
}

void await(std::function<bool()> cond) {
  auto &scheduler = *Scheduler::get_scheduler();
  auto &self = scheduler.get_current_thread_state();

  //
  // "await(cond) подчиняется тем же правилам паузы, что Shared и SchedLock
  // (раздел «Где Scheduler ставит паузу» в этапе 1). Пауза перед ним ставится,
  // если с прошлой паузы поток трогал общее состояние или если cond сейчас ложно."
  //
  if (self.touched || !cond()) {
    self.touched = false;
    self.cond = cond;
    scheduler.yield_to_sched();
    //
    // Если мы здесь, значит, нас разбудил планировщик. Планировщик не будет потоки
    // с cond, если условие cond не выполнено, значит, условие точно выполнено и никем не нарушится,
    // т.к. в момент исполняется только один поток и новых переключений перед началом потока не было.
    //
    self.cond = nullptr;
  }

  self.touched = true;
  scheduler.record("await done");
}

void Scheduler::record(const std::string &str) {
  std::stringstream ss;
  ss << get_thread_id() << " " << str;
  log.push_back(ss.str());
}

void Scheduler::yield_to_sched() {
  ThreadState &current_state = get_current_thread_state();
  sched_semaphore_.release();
  current_state.semaphore.acquire();
}

void Scheduler::pause() {
  ThreadState &current_state = get_current_thread_state();
  if (current_state.touched) {
    current_state.touched = false;
    yield_to_sched();
  }
}

} // namespace sched
