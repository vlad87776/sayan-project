#include <stdint.h>
#include <stddef.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static volatile uint16_t* const VGA_BUFFER = (uint16_t*)0xB8000;

static uint16_t vga_entry(unsigned char ch, uint8_t color) {
    return ((uint16_t)color << 8) | (uint8_t)ch;
}

static void clear_screen(uint8_t color) {
    for (size_t row = 0; row < VGA_HEIGHT; ++row) {
        for (size_t col = 0; col < VGA_WIDTH; ++col) {
            VGA_BUFFER[row * VGA_WIDTH + col] = vga_entry(' ', color);
        }
    }
}

static void write_string_at(const char* s, uint8_t color, uint16_t row, uint16_t col) {
    while (*s && col < VGA_WIDTH) {
        VGA_BUFFER[row * VGA_WIDTH + col] = vga_entry((unsigned char)*s, color);
        ++s;
        ++col;
    }
}

void kernel_main(void) {
    const uint8_t background = 0x1F;  // white on blue

    clear_screen(background);
    write_string_at("Kernel loaded successfully", background, 12, 26);
    write_string_at("Welcome to Sayan OS", background, 13, 30);

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
