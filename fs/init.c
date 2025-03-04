#include <fs.h>
#include <fs/fat16.h>

extern void fat16_init();
void fs_init() { fat16_init(); }