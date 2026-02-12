#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

/* Process states */
typedef enum {
    PROCESS_READY,      /* Ready to run */
    PROCESS_RUNNING,    /* Currently executing */
    PROCESS_BLOCKED,    /* Waiting for I/O or event */
    PROCESS_TERMINATED  /* Finished execution */
} process_state_t;

/* CPU context saved during context switch */
typedef struct {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t rip;       /* Instruction pointer */
    uint64_t rflags;    /* CPU flags */
    uint64_t cr3;       /* Page table base */
} cpu_context_t;

/* Process Control Block (PCB) */
typedef struct process {
    uint32_t pid;                   /* Process ID */
    process_state_t state;          /* Current state */
    cpu_context_t context;          /* Saved CPU state */
    uint64_t* stack;                /* Kernel stack */
    uint64_t stack_size;            /* Stack size */
    uint64_t time_slice;            /* Remaining time slice */
    struct process* next;           /* Next process in queue */
} process_t;

/* Initialize process management */
void process_init(void);

/* Create a new process */
process_t* process_create(void (*entry_point)(void), uint64_t stack_size);

/* Terminate current process */
void process_exit(void);

/* Get current running process */
process_t* process_current(void);

/* Get process by PID */
process_t* process_get(uint32_t pid);

#endif
