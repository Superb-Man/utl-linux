# utl-linux

**utl-linux** (User Thread Library for Linux) is a lightweight, cooperative/preemptive
**user-level (green) threading library** for Linux, written in C. It implements its own
scheduler, thread control blocks, and synchronization primitives (mutex, condition
variable, semaphore) entirely in user space on top of POSIX `ucontext`, without relying
on kernel threads (`pthread`).

## Features

- **User-space thread scheduling** using `ucontext.h` (`getcontext`/`makecontext`/`swapcontext`)
- **Preemptive time-slicing** via `SIGALRM` + `setitimer`, so threads are periodically
  interrupted and rescheduled without needing to yield manually
- **Cooperative yielding** with `uthread_yield()`
- **Thread lifecycle management**: create, exit, join
- **Timed sleep** support (`uthread_sleep`) with efficient wakeup scheduling
- **Synchronization primitives** built from scratch:
  - Mutexes (`uthread_mutex_t`)
  - Condition variables (`uthread_cond_t`)
  - Counting semaphores (`uthread_sem_t`)
- **FIFO ready queue** implemented as a singly linked list scheduler, plus a generic
  linked-list `queue_t` used by the synchronization primitives to track blocked waiters
- Simple `DEBUG_PRINT`/`ERROR_PRINT`/`ASSERT` logging macros (signal-safe, using `write(2)`)

## Repository Layout

```
.
├── Makefile                       # Build rules for the sample programs
├── include/
│   ├── uthread.h / uthread.c      # Core thread API: create, join, exit, yield, sleep, scheduler bootstrap
│   ├── scheduler.h                # Ready queue + schedule_next(): the scheduling loop
│   ├── mutex.h   / mutex.c        # Mutex implementation
│   ├── cond.h    / cond.c         # Condition variable implementation
│   ├── semaphore.h / semaphore.c  # Counting semaphore implementation
│   ├── queue.h   / queue.c        # Generic FIFO linked-list queue used by the above
│   ├── debug.h                    # Logging/assertion macros
│   └── panic.c                    # panic() helper for fatal errors
├── test.c                         # Basic thread creation/join demo (busy-work threads)
├── test2.c                        # Multiple threads cooperatively yielding (prime factorization demo)
├── test3.c                        # Threads using uthread_sleep()
└── test_producer_consumer.c       # Producer/consumer demo using mutexes + semaphores
```

## Requirements

- Linux (relies on `ucontext.h`, `setitimer`, and POSIX signals)
- `gcc` with support for `-fsanitize=address`

## Building

The provided `Makefile` builds the sample test programs:

```sh
make
```

By default this builds the `test3` binary (both `TARGET` and `PC_TARGET` currently point
to `test3` in the `Makefile`). To build a different sample, compile it manually against
the library sources, for example:

```sh
# Build the producer/consumer demo
gcc -fsanitize=address -g test_producer_consumer.c \
    include/uthread.c include/queue.c include/mutex.c include/cond.c include/semaphore.c \
    -o test_producer_consumer

# Build the basic create/join demo
gcc -fsanitize=address -g test.c include/uthread.c include/queue.c -o test
```

Clean up build artifacts with:

```sh
make clean
```

## Usage

Include `include/uthread.h` (and `mutex.h`/`cond.h`/`semaphore.h` as needed) in your
program, then create one or more user threads and start the scheduler:

```c
#include "include/uthread.h"

void worker(void* arg) {
    int id = *(int*)arg;
    printf("Hello from thread %d\n", id);
    uthread_exit(NULL);
}

void main_thread(void* arg) {
    int id = 1;
    int tid = uthread_create(worker, &id);
    uthread_join(tid);
}

int main() {
    uthread_create(main_thread, NULL);
    uthread_run(); // starts the scheduler; does not return until all threads finish
    return 0;
}
```

## API Reference

### Thread management (`include/uthread.h`)

| Function | Description |
|---|---|
| `int uthread_create(void (*start_routine)(void*), void* arg)` | Creates a new thread with its own stack, returns its thread ID or `-1` if no slots are free. |
| `void uthread_exit(void* retval)` | Terminates the calling thread, storing `retval` for a joiner. |
| `void* uthread_join(uthread_t tid)` | Blocks until thread `tid` finishes, then returns its return value. |
| `void uthread_yield(void)` | Voluntarily yields the CPU to another ready thread. |
| `void uthread_sleep(int ms)` | Blocks the calling thread for at least `ms` milliseconds. |
| `uthread_t get_tid(void)` | Returns the ID of the currently running thread. |
| `void uthread_run(void)` | Starts the scheduler loop; call once from `main()` after creating the initial thread(s). |

### Mutex (`include/mutex.h`)

| Function | Description |
|---|---|
| `void uthread_mutex_init(uthread_mutex_t* mutex)` | Initializes a mutex to the unlocked state. |
| `void uthread_mutex_lock(uthread_mutex_t* mutex)` | Acquires the mutex, blocking the calling thread if it is already held. |
| `void uthread_mutex_unlock(uthread_mutex_t* mutex)` | Releases the mutex, waking a waiting thread if any. |

### Condition variable (`include/cond.h`)

| Function | Description |
|---|---|
| `void uthread_cond_init(uthread_cond_t* cond)` | Initializes a condition variable. |
| `void uthread_cond_wait(uthread_cond_t* cond, uthread_mutex_t* mutex)` | Atomically unlocks `mutex` and blocks until signaled, then re-locks `mutex`. |
| `void uthread_cond_signal(uthread_cond_t* cond)` | Wakes one thread waiting on `cond`. |
| `void uthread_cond_broadcast(uthread_cond_t* cond)` | Wakes all threads waiting on `cond`. |

### Semaphore (`include/semaphore.h`)

| Function | Description |
|---|---|
| `void uthread_sem_init(uthread_sem_t* sem, int value)` | Initializes a counting semaphore with the given initial value. |
| `void uthread_sem_wait(uthread_sem_t* sem)` | Decrements the semaphore, blocking if its value is `0`. |
| `void uthread_sem_post(uthread_sem_t* sem)` | Increments the semaphore, waking a blocked thread if any are waiting. |

## Sample Programs

- **`test.c`** — creates two worker threads doing busy-work loops alongside the main
  thread, then joins them.
- **`test2.c`** — spins up several threads that compute prime factors over disjoint
  ranges, cooperatively yielding between iterations.
- **`test3.c`** — demonstrates `uthread_sleep()` by having worker threads sleep before
  printing their results.
- **`test_producer_consumer.c`** — a classic bounded-buffer producer/consumer problem
  using mutexes to protect shared state and semaphores to track empty/full buffer slots.

## Design Notes

- `MAX_THREADS` (64) and `STACK_SIZE` (16 * 4096 bytes) are compile-time constants in
  `include/uthread.h`; increase them if you need more or larger-stacked threads.
- The scheduler (`schedule_next()` in `include/scheduler.h`) wakes sleeping threads whose
  wakeup time has elapsed, dispatches the next ready thread via `swapcontext`, and — when
  no thread is ready but some are blocked — sleeps precisely until the nearest wakeup time
  to avoid busy-waiting, temporarily disabling the periodic timer while doing so.
- `SIGALRM` is blocked (via `block()`/`unblock()` macros wrapping `sigprocmask`) around
  critical sections in the scheduler and synchronization primitives to avoid races with
  preemption.

## Status

This is a learning/research project exploring user-level threading concepts (context
switching, cooperative and preemptive scheduling, and synchronization primitives) on
Linux. Contributions, bug reports, and review feedback are welcome.
