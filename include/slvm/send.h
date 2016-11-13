#ifndef SLVM_SEND_H
#define SLVM_SEND_H

#include "classes/kernel.h"

/**
 * Message send
 */
extern SLVM_Oop slvm_dynrun_csend(int first, ...);

#if defined(__x86_64__)
#define SLVM_SEND(argumentDescription, selector, ...) slvm_dynrun_csend(0, 1, 2, 3, 4, 5, argumentDescription, selector, __VA_ARGS__)
#else
#define SLVM_SEND(argumentDescription, selector, ...) slvm_dynrun_csend(argumentDescription, selector, __VA_ARGS__)
#endif

#endif /* SLVM_SEND_H */
