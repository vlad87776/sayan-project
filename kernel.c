#include <stdint.h>

static volatile uint16_t* const VGA_BUFFER = (uint16_t*)0xB8000;

static void write_string(const char* s, uint8_t color, uint16_t row) {
    uint16_t col = 0;
    while (*s) {
        VGA_BUFFER[row * 80 + col] = ((uint16_t)color << 8) | (uint8_t)(*s);
        ++s;
        ++col;
    }
}

void kernel_main(void) {
    write_string("Simple kernel is running", 0x0F, 0);

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
