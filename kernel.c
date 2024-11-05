#include "uefi.h"

KernelParams kernel_params;
uint64 screen_pointer;

void clear_screen() {
    char *screen = (char *)kernel_params.screen.base;
    for (uint64 i = 0; i < kernel_params.screen.size; i += 4) {
        screen[i] = 0xFF;
        screen[i + 1] = 0xFF;
        screen[i + 2] = 0xFF;
        screen[i + 3] = 0x00;
    }
}

int kernel_main(KernelParams *params) {
    kernel_params.memtotal = params->memtotal;
    kernel_params.screen.base = params->screen.base;
    kernel_params.screen.size = params->screen.size;
    kernel_params.screen.hr = params->screen.hr;
    kernel_params.screen.vr = params->screen.vr;
    clear_screen();
    while (1);
}