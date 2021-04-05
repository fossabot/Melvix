// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <def.h>
#include <serial.h>
#include <str.h>

CLEAR void serial_install(void)
{
	outb(0x3f8 + 1, 0x00);
	outb(0x3f8 + 3, 0x80);
	outb(0x3f8 + 0, 0x03);
	outb(0x3f8 + 1, 0x00);
	outb(0x3f8 + 3, 0x03);
	outb(0x3f8 + 2, 0xC7);
	outb(0x3f8 + 4, 0x0B);
}

static int is_transmit_empty(void)
{
	return inb(0x3f8 + 5) & 0x20;
}

void serial_put(char ch)
{
	while (is_transmit_empty() == 0)
		;
	outb(0x3f8, (u8)ch);
}

void serial_print(const char *data)
{
	for (const char *p = data; p && *p; p++)
		serial_put(*p);
}
