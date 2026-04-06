// syscalls.c
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>

/* Symbols from the linker script */
extern char _end;      // End of .bss, start of heap
extern char _estack;   // Top of RAM

/* Simple heap pointer */
static char *heap_end = 0;

/* malloc support */
caddr_t _sbrk(int incr) {
    char *prev_heap;

    if (heap_end == 0) {
        heap_end = &_end;  // initialize heap start
    }

    prev_heap = heap_end;

    // Prevent heap/stack collision
    if ((heap_end + incr) > &_estack) {
        return (caddr_t)-1; // allocation failed
    }

    heap_end += incr;
    return (caddr_t)prev_heap;
}

/* Minimal syscall stubs for newlib */
int _write(int file, char *ptr, int len) {
    (void)file; (void)ptr; (void)len;
    return len; // pretend we wrote everything
}

int _read(int file, char *ptr, int len) {
    (void)file; (void)ptr; (void)len;
    return 0;
}

int _close(int file) {
    (void)file;
    return -1;
}

int _fstat(int file, struct stat *st) {
    (void)file; (void)st;
    st->st_mode = S_IFCHR; // character device
    return 0;
}

int _lseek(int file, int ptr, int dir) {
    (void)file; (void)ptr; (void)dir;
    return 0;
}

int _isatty(int file) {
    (void)file;
    return 1;
}

void _exit(int status) {
    (void)status;
    while(1); // hang forever
}

void _kill(int pid, int sig) {
    (void)pid; (void)sig;
    while(1); // hang forever
}

int _getpid(void) {
    return 1;
}