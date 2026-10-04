# utl-linux

`utl-linux` is a small, educational user-level threading library for Linux.
It schedules functions within one process using `ucontext` and a periodic
`SIGALRM` timer. The repository also includes mutex, condition-variable, and
semaphore implementations plus example programs.

## Features

- Preemptive, round-robin scheduling with a 10 ms time slice
- Up to 64 thread control blocks (one is reserved for the scheduler context)
- Cooperative yielding with `uthread_yield()`
- Timed suspension with `uthread_sleep()`
- Thread creation, joining, and return-value retrieval
- FIFO wait queues for mutexes, condition variables, and semaphores
- AddressSanitizer-enabled example builds

## Requirements

- Linux or another system that provides `ucontext`, `setitimer`, and POSIX
  signals
- `gcc`
- `make`

The implementation depends on Linux/POSIX facilities and is not portable to
Windows. It is intended for learning and experimentation, not production use.

## Build and run

Build the default scheduler example:

```sh
make
./test
```

The default target builds `test.c` with the threading and synchronization
implementations. To build the producer-consumer demonstration, which exercises
all synchronization primitives, compile it explicitly:

```sh
gcc -fsanitize=address -g test_producer_consumer.c \
  include/uthread.c include/queue.c include/mutex.c \
  include/cond.c include/semaphore.c -o producer_consumer
./producer_consumer
```

Remove generated binaries with:

```sh
make clean
rm -f producer_consumer
```

## Basic usage

Include `include/uthread.h`, create one or more threads, and then start the
scheduler:

```c
#include "include/uthread.h"

static void worker(void *arg) {
    /* do work */
}

int main(void) {
    uthread_create(worker, NULL);
    uthread_run();
}
```

Thread entry functions receive a `void *` argument and do not return a value.
They finish when they return, or may call `uthread_exit(retval)` explicitly.
Call `uthread_join(tid)` from another managed thread to wait for completion
and receive that value.

## Public API

### Thread lifecycle

| Function | Description |
| --- | --- |
| `uthread_create(start_routine, arg)` | Creates a ready thread and returns its ID, or `-1` when no thread slot is available. |
| `uthread_run()` | Initializes the scheduler and begins scheduling created threads. |
| `uthread_yield()` | Places the current thread back on the ready queue and schedules another ready thread. |
| `uthread_sleep(ms)` | Blocks the current thread until at least `ms` milliseconds have elapsed. |
| `uthread_join(tid)` | Blocks until `tid` exits, releases its stack, and returns its exit value. |
| `uthread_exit(retval)` | Marks the current thread complete and makes `retval` available to its joiner. |
| `get_tid()` | Returns the current managed thread's ID. |

### Synchronization

| Type | Initialization | Operations |
| --- | --- | --- |
| `uthread_mutex_t` | `uthread_mutex_init()` | `uthread_mutex_lock()`, `uthread_mutex_unlock()` |
| `uthread_cond_t` | `uthread_cond_init()` | `uthread_cond_wait()`, `uthread_cond_signal()`, `uthread_cond_broadcast()` |
| `uthread_sem_t` | `uthread_sem_init(sem, value)` | `uthread_sem_wait()`, `uthread_sem_post()` |

Initialize each synchronization object before use. A thread calling
`uthread_cond_wait()` must hold the mutex passed to it; the function releases
that mutex while blocked and locks it again before returning. As with POSIX
condition variables, test the associated predicate in a loop after waking.

## How it works

Thread state is held in a fixed `thread_table`. Each created thread receives an
aligned 64 KiB stack and a `ucontext` configured to start in an internal
wrapper. Ready threads are linked in a FIFO scheduler queue. The interval
timer invokes the scheduler through `SIGALRM`, while blocking operations mark
the current thread as blocked and switch to the next ready thread.

Mutexes, condition variables, and semaphores keep their own FIFO waiter
queues. Their operations temporarily block `SIGALRM` while updating shared
scheduler and wait-queue state, preventing timer-driven context switches in
those critical sections.

## Repository layout

| Path | Purpose |
| --- | --- |
| `include/uthread.[ch]` | Thread API, lifecycle management, timer, and context switching |
| `include/scheduler.h` | Ready-queue scheduler implementation |
| `include/queue.[ch]` | Generic FIFO queue used by synchronization primitives |
| `include/mutex.[ch]` | User-level mutex |
| `include/cond.[ch]` | User-level condition variable |
| `include/semaphore.[ch]` | Counting semaphore |
| `test.c`, `test2.c`, `test3.c` | Scheduler and yielding examples |
| `test_producer_consumer.c` | Bounded-buffer synchronization demonstration |

## Limitations

- The fixed table supports at most 63 created threads at once.
- A thread can have only one recorded joiner; joining does not guard against
  multiple join attempts.
- Context-switching and synchronization operations must be called from
  managed threads after `uthread_run()` starts the scheduler.
- The scheduler terminates the process when no managed threads remain active.
- The code uses signal handlers and `ucontext`, so it should be treated as an
  educational implementation with carefully controlled workloads.
