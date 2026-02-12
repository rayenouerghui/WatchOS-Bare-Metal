#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

/* Initialize scheduler */
void scheduler_init(void);

/* Add process to ready queue */
void scheduler_add(process_t* proc);

/* Remove process from ready queue */
void scheduler_remove(process_t* proc);

/* Get next process to run (round-robin) */
process_t* scheduler_next(void);

/* Perform context switch (called from timer interrupt) */
void scheduler_switch(void);

/* Yield CPU to next process */
void scheduler_yield(void);

#endif
