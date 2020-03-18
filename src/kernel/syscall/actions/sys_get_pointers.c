#include <stdint.h>
#include <kernel/graphics/vesa.h>
#include <kernel/fs/load.h>
#include <kernel/memory/alloc.h>

struct userspace_pointers {
	unsigned char *fb;
	struct font *font;
};

uint32_t sys_get_pointers()
{
	struct userspace_pointers *pointers =
		(struct userspace_pointers *)kmalloc(sizeof(struct userspace_pointers));
	pointers->fb = fb;
	pointers->font = font;
	return (uint32_t)pointers;
}