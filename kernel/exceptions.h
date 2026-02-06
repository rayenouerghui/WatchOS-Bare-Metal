#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdint.h>

/* Exception handler called from assembly */
void exception_handler(uint64_t int_no, uint64_t err_code);

#endif
