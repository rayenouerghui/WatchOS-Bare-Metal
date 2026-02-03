#ifndef KPRINT_H
#define KPRINT_H

#include <stdint.h>

/* Kernel print functions - structured logging for the kernel */

/* Log levels */
typedef enum {
    LOG_INFO,    /* Informational messages (white) */
    LOG_OK,      /* Success messages (green) */
    LOG_WARN,    /* Warning messages (yellow) */
    LOG_ERROR    /* Error messages (red) */
} log_level_t;

/* Initialize kernel printing system */
void kprint_init(void);

/* Print a kernel message with specified log level */
void kprint(log_level_t level, const char* message);

/* Convenience wrappers */
void kprint_info(const char* message);
void kprint_ok(const char* message);
void kprint_warn(const char* message);
void kprint_error(const char* message);

/* Print hex value */
void kprint_hex(uint8_t value);

#endif
