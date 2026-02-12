#include "process.h"
#include "heap.h"
#include "panic.h"
#include "kprint.h"

#define MAX_PROCESSES 64
#define DEFAULT_STACK_SIZE 8192  /* 8KB stack per process */

static process_t* process_table[MAX_PROCESSES];
static process_t* current_process = NULL;
static uint32_t next_pid = 1;

void process_init(void) {
    /* Clear process table */
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i] = NULL;
    }
    
    /* Create idle process (PID 0) */
    current_process = heap_alloc(sizeof(process_t));
    current_process->pid = 0;
    current_process->state = PROCESS_RUNNING;
    current_process->stack = NULL;  /* Kernel uses its own stack */
    current_process->stack_size = 0;
    current_process->time_slice = 0;
    current_process->next = NULL;
    
    process_table[0] = current_process;
    
    kprint_ok("Process management initialized");
}

process_t* process_create(void (*entry_point)(void), uint64_t stack_size) {
    if (next_pid >= MAX_PROCESSES) {
        kprint_error("Process table full");
        return NULL;
    }
    
    /* Allocate PCB */
    process_t* proc = heap_alloc(sizeof(process_t));
    if (!proc) {
        panic("Failed to allocate PCB");
    }
    
    /* Allocate stack */
    proc->stack = heap_alloc(stack_size);
    if (!proc->stack) {
        heap_free(proc);
        panic("Failed to allocate process stack");
    }
    
    /* Initialize PCB */
    proc->pid = next_pid++;
    proc->state = PROCESS_READY;
    proc->stack_size = stack_size;
    proc->time_slice = 10;  /* 10 timer ticks */
    proc->next = NULL;
    
    /* Set up initial context */
    proc->context.rip = (uint64_t)entry_point;
    proc->context.rsp = (uint64_t)proc->stack + stack_size - 16;  /* Stack grows down */
    proc->context.rbp = proc->context.rsp;
    proc->context.rflags = 0x202;  /* Interrupts enabled */
    
    /* Zero out other registers */
    proc->context.rax = proc->context.rbx = proc->context.rcx = proc->context.rdx = 0;
    proc->context.rsi = proc->context.rdi = 0;
    proc->context.r8 = proc->context.r9 = proc->context.r10 = proc->context.r11 = 0;
    proc->context.r12 = proc->context.r13 = proc->context.r14 = proc->context.r15 = 0;
    
    /* Add to process table */
    process_table[proc->pid] = proc;
    
    return proc;
}

void process_exit(void) {
    if (!current_process || current_process->pid == 0) {
        panic("Cannot exit idle process");
    }
    
    current_process->state = PROCESS_TERMINATED;
    
    /* Free resources */
    if (current_process->stack) {
        heap_free(current_process->stack);
    }
    
    process_table[current_process->pid] = NULL;
    
    /* Switch to next process (scheduler will handle this) */
}

process_t* process_current(void) {
    return current_process;
}

process_t* process_get(uint32_t pid) {
    if (pid >= MAX_PROCESSES) {
        return NULL;
    }
    return process_table[pid];
}
