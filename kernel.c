#include <stdint.h>
#include <stddef.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define COLOR_BLACK 0x0
#define COLOR_WHITE 0xF
#define COLOR_BLUE 0x1
#define COLOR_CYAN 0x3
#define COLOR_YELLOW 0xE

static volatile uint16_t* const VGA_BUFFER = (uint16_t*)0xB8000;

typedef enum {
    MENU_BOOT = 0,
    MENU_REBOOT = 1,
    MENU_HALT = 2,
    MENU_COUNT = 3
} menu_item_t;

static inline void io_wait(void) {
    __asm__ volatile ("outb %%al, $0x80" : : "a"(0));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

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

static void draw_boot_menu(menu_item_t selected) {
    const uint8_t bg = (COLOR_WHITE << 4) | COLOR_BLUE;
    const uint8_t title = (COLOR_YELLOW << 4) | COLOR_BLUE;
    const uint8_t normal = (COLOR_WHITE << 4) | COLOR_BLUE;
    const uint8_t active = (COLOR_BLACK << 4) | COLOR_CYAN;

    clear_screen(bg);

    write_string_at("=== SAYAN OS BOOT MENU ===", title, 4, 26);
    write_string_at("Use W/S or Arrow Up/Down + Enter", normal, 6, 22);

    write_string_at(selected == MENU_BOOT ? "> 1) Boot SAYAN OS" : "  1) Boot SAYAN OS",
                    selected == MENU_BOOT ? active : normal, 10, 26);
    write_string_at(selected == MENU_REBOOT ? "> 2) Reboot" : "  2) Reboot",
                    selected == MENU_REBOOT ? active : normal, 12, 26);
    write_string_at(selected == MENU_HALT ? "> 3) Halt" : "  3) Halt",
                    selected == MENU_HALT ? active : normal, 14, 26);

    write_string_at("Enter = select", normal, 20, 31);
}

static void show_boot_sequence(void) {
    const uint8_t bg = (COLOR_WHITE << 4) | COLOR_BLUE;
    const uint8_t text = (COLOR_WHITE << 4) | COLOR_BLUE;

    clear_screen(bg);
    write_string_at("SAYAN OS is loading...", text, 8, 29);
    write_string_at("[OK] Kernel core", text, 10, 30);
    write_string_at("[OK] VGA driver", text, 11, 30);
    write_string_at("[OK] Input subsystem (polling)", text, 12, 30);
    write_string_at("[OK] Boot menu", text, 13, 30);
    write_string_at("System is ready.", text, 15, 31);
}

static void reboot_system(void) {
    while (inb(0x64) & 0x02) {
        io_wait();
    }
    outb(0x64, 0xFE);
}

static void halt_system(void) {
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

static menu_item_t next_item(menu_item_t item) {
    return (menu_item_t)(((int)item + 1) % MENU_COUNT);
}

static menu_item_t prev_item(menu_item_t item) {
    return (menu_item_t)(((int)item + MENU_COUNT - 1) % MENU_COUNT);
}

void kernel_main(void) {
    menu_item_t selected = MENU_BOOT;
    uint8_t extended = 0;

    draw_boot_menu(selected);

    for (;;) {
        if (!(inb(0x64) & 0x01)) {
            continue;
        }

        uint8_t scancode = inb(0x60);

        if (scancode == 0xE0) {
            extended = 1;
            continue;
        }

        if (scancode & 0x80) {
            extended = 0;
            continue;
        }

        if ((!extended && scancode == 0x11) || (extended && scancode == 0x48)) {
            selected = prev_item(selected);
            draw_boot_menu(selected);
        } else if ((!extended && scancode == 0x1F) || (extended && scancode == 0x50)) {
            selected = next_item(selected);
            draw_boot_menu(selected);
        } else if (scancode == 0x1C) {
            if (selected == MENU_BOOT) {
                show_boot_sequence();
            } else if (selected == MENU_REBOOT) {
                reboot_system();
                halt_system();
            } else {
                clear_screen((COLOR_WHITE << 4) | COLOR_BLUE);
                write_string_at("System halted.", (COLOR_WHITE << 4) | COLOR_BLUE, 12, 33);
                halt_system();
            }
        }

        extended = 0;
    }
}
