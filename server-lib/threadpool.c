#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "threadpool.h"

int create_task_list(task_list_t *task_list, int polling_delay) {
  task_list->n_tasks = 0;
  task_list->first_task = NULL;
  task_list->last_task = NULL;
  task_list->should_stop = 0;
  task_list->polling_delay = polling_delay;

  task_list->tasks_lock = malloc(sizeof(pthread_mutex_t));
  if (pthread_mutex_init(task_list->tasks_lock, NULL) != 0) {
    return 1;
  }

  task_list->stop_lock = malloc(sizeof(pthread_mutex_t));
  if (pthread_mutex_init(task_list->tasks_lock, NULL) != 0) {
    return 1;
  }

  return 0;
}

int add_task_to_task_list(task_t *task, task_list_t *task_list) {
  pthread_mutex_lock(task_list->tasks_lock);

  if (task_list->n_tasks > 0) {
    task_list->last_task->next = task;
  } else {
    task_list->first_task = task;
  }

  task_list->last_task = task;
  task_list->n_tasks += 1;

  pthread_mutex_unlock(task_list->tasks_lock);

  return 0;
}

int take_task_from_task_list(task_t *task, task_list_t *task_list) {
  pthread_mutex_lock(task_list->tasks_lock);

  int found_valid_task = -1;
  if (task_list->n_tasks > 0) {
    task_t *first_task = task_list->first_task;
    memcpy(task, first_task, sizeof(task_t));

    task_list->n_tasks -= 1;
    task_list->first_task = first_task->next;
    free(first_task);

    if (task_list->n_tasks == 0) {
      task_list->first_task = NULL;
      task_list->last_task = NULL;
    }

    found_valid_task = 0;
  }

  pthread_mutex_unlock(task_list->tasks_lock);

  return found_valid_task;
}

int stop_task_list(task_list_t *task_list) {
  pthread_mutex_lock(task_list->stop_lock);
  task_list->should_stop = 1;
  pthread_mutex_unlock(task_list->stop_lock);

  return 0;
}

int should_stop(task_list_t *task_list) {
  pthread_mutex_lock(task_list->stop_lock);
  int should_stop = task_list->should_stop;

  if (should_stop) {
    pthread_mutex_lock(task_list->tasks_lock);
    should_stop = task_list->n_tasks == 0;
    pthread_mutex_unlock(task_list->tasks_lock);
  }

  pthread_mutex_unlock(task_list->stop_lock);

  return should_stop;
}

int destroy_task_list(task_list_t *task_list) {
  task_t *task;

  // FIX This is very inefficient, should be a custom remove function
  // FIX Current implementation also does not free the last task
  while (task_list->n_tasks > 0) {
    take_task_from_task_list(task, task_list);
  }

  pthread_mutex_destroy(task_list->tasks_lock);
  free(task_list->tasks_lock);

  pthread_mutex_destroy(task_list->stop_lock);
  free(task_list->stop_lock);

  return 0;
}

void *run_thread(void *arg) {
  task_list_t *task_list = (task_list_t *)arg;

  task_t *task = malloc(sizeof(task_t));

  while (!should_stop(task_list)) {
    if (take_task_from_task_list(task, task_list) == 0) {
      task->task_executor(task->input_arguments);
    } else {
      usleep(task_list->polling_delay);
    }
  }

  free(task);

  return NULL;
}

int create_threadpool(int n_threads, int polling_delay,
                      threadpool_t *threadpool) {
  threadpool->n_threads = n_threads;
  threadpool->threads = malloc(sizeof(pthread_t) * n_threads);

  threadpool->task_list = malloc(sizeof(task_list_t));
  create_task_list(threadpool->task_list, polling_delay);

  for (size_t i = 0; i < n_threads; i++) {
    pthread_create(&threadpool->threads[i], NULL, run_thread,
                   threadpool->task_list);
  }

  return 0;
}

int add_task_to_threadpool(void (*task_executor)(void *input_arguments),
                           void *input_arguments, threadpool_t *threadpool) {
  task_t *task = malloc(sizeof(task_t));
  task->task_executor = task_executor;
  task->input_arguments = input_arguments;
  task->next = NULL;

  add_task_to_task_list(task, threadpool->task_list);

  return 0;
}

int destroy_threadpool(threadpool_t *threadpool) {
  stop_task_list(threadpool->task_list);

  for (size_t i = 0; i < threadpool->n_threads; i++) {
    pthread_join(threadpool->threads[i], NULL);
  }
  free(threadpool->threads);

  destroy_task_list(threadpool->task_list);
  free(threadpool->task_list);

  return 0;
}
