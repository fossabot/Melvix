// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <def.h>
#include <gui.h>
#include <input.h>
#include <print.h>
#include <sys.h>
#include <vesa.h>

void onkey(u32 scancode)
{
	printf("KEY EVENT %d\n", scancode);
	if (KEY_ALPHANUMERIC(scancode)) {
		printf("ALPHANUMERIC!\n");
	}
}

void main(char **argv)
{
	struct vbe *vbe = (struct vbe *)argv[0];

	print("WM loaded.\n");
	printf("VBE: %dx%d\n", vbe->width, vbe->height);

	const u32 color[3] = { 0xff, 0xff, 0 };
	const u32 text[3] = { 0, 0, 0 };
	vesa_fill(vbe, color);
	gui_init("/font/spleen-16x32.psfu");
	gui_write(vbe, 50, 50, text, "hallo");

	printf("onkey: %x\n", onkey);
	map(EVENT_KEYBOARD, onkey);

	while (1) {
	};
	exit();
}
