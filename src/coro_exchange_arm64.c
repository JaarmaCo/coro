#include "coro-impl.h"

__asm__ (
".global coro_exchange_\n"
"coro_exchange_:\n"
	"str x30, [x0, #0]\n\t" // Store return address

	// Store stack pointer
	"mov x2, sp\n\t"
	"str x2, [x0, #8]\n\t" 

	// Store misc. registers
	"str x19, [x0, #16]\n\t"
	"str x20, [x0, #24]\n\t"
	"str x21, [x0, #32]\n\t"
	"str x22, [x0, #40]\n\t"
	"str x23, [x0, #48]\n\t"
	"str x24, [x0, #56]\n\t"
	"str x25, [x0, #64]\n\t"
	"str x26, [x0, #72]\n\t"
	"str x27, [x0, #80]\n\t"
	"str x28, [x0, #88]\n\t"
	"str x29, [x0, #96]\n\t"

	// Restore misc. registers
	"ldr x19, [x1, #16]\n\t"
	"ldr x20, [x1, #24]\n\t"
	"ldr x21, [x1, #32]\n\t"
	"ldr x22, [x1, #40]\n\t"
	"ldr x23, [x1, #48]\n\t"
	"ldr x24, [x1, #56]\n\t"
	"ldr x25, [x1, #64]\n\t"
	"ldr x26, [x1, #72]\n\t"
	"ldr x27, [x1, #80]\n\t"
	"ldr x28, [x1, #88]\n\t"
	"ldr x29, [x1, #96]\n\t"

	// Restore stack
	"ldr x2, [x1, #8]\n\t"
	"mov sp, x2\n\t"

	// Execute
	"ldr x2, [x1, #0]\n\t"
	"br x2\n\t"
);

