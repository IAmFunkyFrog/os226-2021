#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include "sched.h"
#include "pool.h"

typedef struct sched_node *(*polling_function)(struct sched_node **);
typedef int (*cmp_function)(struct sched_node *, struct sched_node*);

struct sched_node {
    void (*entrypoint)(void *aspace);

    void *aspace;
    int priority;
    int deadline;
    int timeout;

    struct sched_node *next;
    struct sched_node *previous;
};

struct sched_node *schedule_waiting = NULL;
struct sched_node *schedule_ready = NULL;
struct sched_node *executing_task = NULL;
static int time;

struct sched_node *insert_in_schedule(struct sched_node *schedule, struct sched_node *node) {
    if(schedule == NULL) {
        node->next = schedule;
        node->previous = NULL;
        schedule = node;
    }
    else {
        struct sched_node *last;
        for (last = schedule; last->next != NULL; last = last->next);
        last->next = node;
        node->previous = last;
        node->next = NULL;
    }
    return schedule;
}

struct sched_node *remove_from_schedule(struct sched_node *schedule, struct sched_node *node) {
    if (schedule == node) {
        if (schedule->next != NULL) schedule->next->previous = NULL;
        return schedule->next;
    } else {
        for (struct sched_node *it = schedule; it != NULL; it = it->next) {
            if (node == it) {
                if (it->next != NULL) it->next->previous = it->previous;
                if (it->previous != NULL) it->previous->next = it->next;
            }
        }
        return schedule;
    }
}

void sched_new(void (*entrypoint)(void *aspace),
               void *aspace,
               int priority,
               int deadline) {
    struct sched_node *new_node = malloc(sizeof(struct sched_node));
    *new_node = (struct sched_node) {
            .entrypoint = entrypoint,
            .aspace = aspace,
            .priority = priority,
            .deadline = deadline,
            .next = NULL,
            .previous = NULL
    };
    schedule_ready = insert_in_schedule(schedule_ready, new_node);
}

void sched_cont(void (*entrypoint)(void *aspace),
                void *aspace,
                int timeout) {
    if (executing_task->entrypoint == entrypoint && executing_task->aspace == aspace) {
        struct sched_node *copied_executing_task = malloc(sizeof(struct sched_node));
        memcpy(copied_executing_task, executing_task, sizeof(struct sched_node));
        copied_executing_task->timeout = time + timeout;

        schedule_waiting = insert_in_schedule(schedule_waiting, copied_executing_task);
        sched_time_elapsed(0);
        return;
    }
    fprintf(stderr, "sched_cont must be used on executing task");
}

void sched_time_elapsed(unsigned amount) {
    time += amount;

    for (struct sched_node *node = schedule_waiting; node != NULL; node = node->next) {
        if (time >= node->timeout) {
            struct sched_node *ready_node = node;
            node = node->next;
            schedule_waiting = remove_from_schedule(schedule_waiting, ready_node);
            schedule_ready = insert_in_schedule(schedule_ready, ready_node);
        }
        if (node == NULL) return;
    }
}

static int cmp_by_round_robin(struct sched_node *n1, struct sched_node *n2) {
    return -1;
}

static int cmp_by_prio(struct sched_node *n1, struct sched_node *n2) {
    return n2->priority - n1->priority;
}

static int cmp_by_deadline(struct sched_node *n1, struct sched_node *n2) {
    if(n1->deadline >= 0 && n2->deadline >= 0) {
        if(n1->deadline != n2->deadline) return n1->deadline - n2->deadline;
        else return cmp_by_prio(n1, n2);
    }
    else if(n1->deadline < 0 && n2->deadline < 0) return cmp_by_prio(n1, n2);
    else if(n1->deadline < 0) return 1;
    else if(n2->deadline < 0) return -1;
}

static void sched_run_with_cmp(cmp_function cmp) {
    while (schedule_ready != NULL) {
        executing_task = schedule_ready;
        for(struct sched_node *node = schedule_ready->next; node != NULL; node = node->next) {
            if(cmp(executing_task, node) > 0) executing_task = node;
        }
        schedule_ready = remove_from_schedule(schedule_ready, executing_task);
        executing_task->entrypoint(executing_task->aspace);
        free(executing_task);
        executing_task = NULL;
    }
}

void sched_run(enum policy policy) {
    switch (policy) {
        case POLICY_FIFO:
            sched_run_with_cmp(cmp_by_round_robin);
            break;
        case POLICY_PRIO:
            sched_run_with_cmp(cmp_by_prio);
            break;
        case POLICY_DEADLINE:
            sched_run_with_cmp(cmp_by_deadline);
            break;
        default:
            fprintf(stderr, "Unknown policy given for sched_run");
            exit(-1);
    }
}
