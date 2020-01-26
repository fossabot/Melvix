#include <kernel/timer/timer.h>
#include <kernel/io/io.h>
#include <kernel/graphics/vesa.h>
#include <kernel/lib/string.h>
#include <kernel/lib/stdlib.h>
#include <kernel/memory/paging.h>
#include <kernel/lib/stdio.h>
#include <stdarg.h>

uint32_t initial_esp;

char *vga_buffer = (char *) 0x500;

void vga_clear()
{
    uint16_t *terminal_buffer = (uint16_t *) 0xB8000;
    for (size_t y = 0; y < 25; y++)
        for (size_t x = 0; x < 80; x++)
            terminal_buffer[y * 80 + x] = 0 | (uint16_t) 0x700;
}

static int line = 0;

void vga_log(char *msg)
{
    if (line == 0) vga_clear();
    uint16_t *terminal_buffer = (uint16_t *) 0xB8000;
    for (size_t i = 0; i < strlen(msg); i++)
        terminal_buffer[line * 80 + i] = (uint16_t) msg[i] | (uint16_t) 0x700;
    char string[80];
    strcpy(string, "[");
    strcat(string, itoa((int) get_time()));
    strcat(string, "] ");
    strcat(string, "INF: ");
    strcat(string, msg);
    strcat(string, "\n");
    strcat(vga_buffer, string);
    line++;
}

void kernel_time()
{
    printf("[%d] ", (int) get_time());
}

void debug(const char *fmt, ...)
{
    vesa_set_color(vesa_dark_white);
    kernel_time();
    printf("DBG: ");

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    vesa_set_color(default_text_color);
    writec('\n');
}

void info(const char *fmt, ...)
{
    vesa_set_color(vesa_blue);
    kernel_time();
    printf("INF: ");

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    vesa_set_color(default_text_color);
    writec('\n');
}

void warn(const char *fmt, ...)
{
    vesa_set_color(vesa_dark_yellow);
    kernel_time();
    printf("WRN: ");

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    vesa_set_color(default_text_color);
    writec('\n');
}

const char *random_message[10] = {
        "Uh... Did I do that?",
        "Layer 8 problem!",
        "Oops.",
        "DON'T PANIC!",
        "Must be a typo.",
        "I'm tired of this ;(",
        "PC LOAD LETTER",
        "Have you tried turning it off and on again?",
        "Call 01189998819991197253 pls",
        "Please fix me!"
};

void panic(char *msg)
{
    asm ("cli");
    vesa_set_color(vesa_dark_red);
    kernel_time();
    serial_printf("PNC: %s - System halted!", msg);
    printf("PNC: %s - System halted!\n\n", msg);
    printf("> %s", random_message[get_time() % 10]);
    halt_loop();
}

void assert(int x)
{
    if (x == 0) {
        panic("Assertion failed");
    }
}

void halt_loop()
{
    asm ("cli");
    loop:
    asm ("hlt");
    goto loop;
}

void v86(uint8_t code, regs16_t *regs)
{
    paging_disable();
    int32(code, regs);
    paging_enable();
}