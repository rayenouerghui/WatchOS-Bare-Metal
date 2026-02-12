#include "scheduler.h"
#include "process.h"
#include "panic.h"
#include "kprint.h"

#include <stddef.h>

/* Ready queue (circular linked list) */
static process_t* current_process = NULL;
static process_t* ready_queue_head = NULL;
static process_t* ready_queue_tail = NULL;

/* Context switch assembly function */
extern void context_switch(cpu_context_t* old_ctx, cpu_context_t* new_ctx);

void scheduler_init(void) {
    ready_queue_head = NULL;
    ready_queue_tail = NULL;
    kprint_ok("Scheduler initialized (round-robin)");
}

void scheduler_add(process_t* proc) {
    if (!proc) return;
    
    proc->state = PROCESS_READY;
    proc->next = NULL;
    
    if (!ready_queue_head) {
        /* Queue is empty */
        ready_queue_head = proc;
        ready_queue_tail = proc;
        proc->next = proc;  /* Circular */
    } else {
        /* Add to tail */
        ready_queue_tail->next = proc;
        proc->next = ready_queue_head;  /* Make circular */
        ready_queue_tail = proc;
    }
}

void scheduler_remove(process_t* proc) {
    if (!proc || !ready_queue_head) return;
    
    /* Single process in queue */
    if (ready_queue_head == ready_queue_tail && ready_queue_head == proc) {
        ready_queue_head = NULL;
        ready_queue_tail = NULL;
        return;
    }
    
    /* Find and remove */
    process_t* current = ready_queue_head;
    process_t* prev = ready_queue_tail;
    
    do {
        if (current == proc) {
            prev->next = current->next;
            
            if (current == ready_queue_head) {
                ready_queue_head = current->next;
            }
            if (current == ready_queue_tail) {
                ready_queue_tail = prev;
            }
            
            current->next = NULL;
            return;
        }
        prev = current;
        current = current->next;
    } while (current != ready_queue_head);
}

process_t* scheduler_next(void) {
    if (!ready_queue_head) {
        return NULL;  /* No processes ready */
    }
    
    /* Round-robin: get head and rotate */
    process_t* next = ready_queue_head;
    ready_queue_head = ready_queue_head->next;
    ready_queue_tail = next;
    
    return next;
}

void scheduler_switch(void) {
    process_t* current = process_current();
    
    /* Decrement time slice */
    if (current && current->time_slice > 0) {
        current->time_slice--;
    }
    
    /* Check if time slice expired or process blocked */
    if (!current || current->time_slice == 0 || current->state != PROCESS_RUNNING) {
        process_t* next = scheduler_next();
        
        if (!next) {
            return;  /* No other process to run */
        }
        
        /* Save current process state */
        if (current && current->state == PROCESS_RUNNING) {
            current->state = PROCESS_READY;
            current->time_slice = 10;  /* Reset time slice */
            scheduler_add(current);
        }
        
        /* Switch to next process */
        next->state = PROCESS_RUNNING;
        
        /* Perform context switch */
        if (current) {
            context_switch(&current->context, &next->context);
        }
    }
}

void scheduler_yield(void) {
    process_t* current = process_current();
    if (current) {
        current->time_slice = 0;  /* Force switch */
    }
    scheduler_switch();
}
