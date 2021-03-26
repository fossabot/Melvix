// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <def.h>
#include <errno.h>
#include <input.h>
#include <ioctl.h>
#include <libgui/gfx.h>
#include <libgui/gui.h>
#include <libgui/msg.h>
#include <libgui/vesa.h>
#include <libtxt/keymap.h>
#include <list.h>
#include <random.h>

//#define FLUSH_TIMEOUT 6

struct client {
	u32 pid;
};

struct window {
	u32 id;
	u32 shid;
	const char *name;
	struct context ctx;
	struct client client;
	u32 flags;
	vec2 pos;
	vec2 pos_prev;
};

struct rectangle {
	vec2 pos1; // Upper left
	vec2 pos2; // Lower right
	void *data;
};

static u8 bypp = 4;
static struct vbe screen = { 0 };
static struct list *windows = NULL; // THIS LIST SHALL BE SORTED BY Z-INDEX!
static struct window *direct = NULL;
static struct window *root = NULL;
static struct window *wallpaper = NULL;
static struct window *cursor = NULL;
static struct window *focused = NULL;
static struct keymap *keymap = NULL;
static struct client wm_client = { 0 };
static struct {
	u8 shift : 1;
	u8 alt : 1;
	u8 ctrl : 1;
} special_keys = { 0 };
static struct {
	vec2 pos;
	u8 left : 1;
	u8 mid : 1;
	u8 right : 1;
} mouse = { 0 };

static void buffer_flush(void)
{
#ifdef FLUSH_TIMEOUT
	static u32 time_flush = 0;
	u32 time_now = time();
	if (time_now - time_flush > FLUSH_TIMEOUT) {
		memcpy(direct->ctx.fb, root->ctx.fb, root->ctx.bytes);
		time_flush = time_now;
	}
#else
	memcpy(direct->ctx.fb, root->ctx.fb, root->ctx.bytes);
#endif
}

/**
 * 5head algorithms
 * Thanks to @LarsVomMars for the help
 */

static void windows_at_rec(vec2 pos1, vec2 pos2, struct list *list)
{
	u32 width = pos2.x - pos1.x;
	u32 height = pos2.y - pos1.y;
	vec2 rec_corners[] = {
		pos1,
		vec2_add(pos1, vec2(width, 0)),
		pos2,
		vec2_add(pos1, vec2(0, height)),
	};

	struct node *iterator = windows->head;
	while (iterator) {
		struct window *win = iterator->data;
		if ((win->flags & WF_NO_WINDOW) != 0)
			goto next;

		vec2 corners[] = {
			win->pos,
			vec2_add(win->pos, vec2(win->ctx.size.x, 0)),
			vec2_add(win->pos, vec2(win->ctx.size.x, win->ctx.size.y)),
			vec2_add(win->pos, vec2(0, win->ctx.size.y)),
		};

		for (int i = 0; i < 4; i++) {
			vec2 corner = corners[i];
			if ((pos1.x < corner.x && pos1.y < corner.y) &&
			    (pos2.x > corner.x && pos2.y > corner.y)) {
				list_add(list, win);
				goto next;
			}
		}

		vec2 win_pos1 = win->pos;
		vec2 win_pos2 = vec2_add(win->pos, win->ctx.size);
		for (int i = 0; i < 4; i++) {
			vec2 corner = rec_corners[i];
			if ((win_pos1.x < corner.x && win_pos1.y < corner.y) &&
			    (win_pos2.x > corner.x && win_pos2.y > corner.y)) {
				list_add(list, win);
				goto next;
			}
		}

	next:
		iterator = iterator->next;
	}
}

static struct rectangle rectangle_at(vec2 pos1, vec2 pos2, struct window *excluded)
{
	u32 width = pos2.x - pos1.x;
	u32 height = pos2.y - pos1.y;
	u32 pitch = width * bypp;
	u8 *data = zalloc(width * height * bypp);

	struct list *windows_at = list_new();
	windows_at_rec(pos1, pos2, windows_at);
	struct node *iterator = windows_at->head;
	while (iterator) {
		struct window *win = iterator->data;
		iterator = iterator->next;

		if (win == excluded)
			continue;

		s32 start_x = win->pos.x - pos1.x;
		u32 end_x = width;
		if (start_x <= 0) { // Either right side or background
			u32 right = start_x + win->ctx.size.x;
			if (right <= width) { // Right side
				start_x = 0;
				end_x = right;
			} else { // Background
				start_x = 0;
			}
		}

		s32 start_y = win->pos.y - pos1.y;
		u32 end_y = height;
		if (start_y <= 0) { // Either bottom side or background
			u32 bot = start_y + win->ctx.size.y;
			if (bot <= height) { // Bottom side
				start_y = 0;
				end_y = bot;
			} else { // Background
				start_y = 0;
			}
		}

		vec2 pos = vec2_sub(pos1, win->pos);

		u8 *srcfb =
			&win->ctx.fb[(pos.x + start_x) * bypp + (pos.y + start_y) * win->ctx.pitch];
		u8 *destfb = &data[start_x * bypp + start_y * pitch];

		// Copy window data to rectangle buffer
		for (u32 cy = start_y; cy < end_y; cy++) {
			u32 diff = 0;
			for (u32 cx = start_x; cx < end_x; cx++) {
				if (srcfb[bypp - 1])
					memcpy(destfb, srcfb, bypp);

				srcfb += bypp;
				destfb += bypp;
				diff += bypp;
			}
			srcfb += win->ctx.pitch - diff;
			destfb += pitch - diff;
		}
	}
	list_destroy(windows_at);

	return (struct rectangle){ .pos1 = pos1, .pos2 = pos2, .data = data };
}

static void rectangle_redraw(vec2 pos1, vec2 pos2, struct window *excluded)
{
	struct rectangle rec = rectangle_at(pos1, pos2, excluded);

	u8 *srcfb = rec.data;
	u8 *destfb = &root->ctx.fb[rec.pos1.x * bypp + rec.pos1.y * root->ctx.pitch];
	for (u32 cy = 0; cy < excluded->ctx.size.y; cy++) {
		memcpy(destfb, srcfb, excluded->ctx.size.x * bypp);
		srcfb += excluded->ctx.pitch;
		destfb += root->ctx.pitch;
	}

	free(rec.data);

	gfx_ctx_on_ctx(&root->ctx, &excluded->ctx, excluded->pos);
}

/**
 * Window operations
 */

static struct window *window_new(struct client client, const char *name, struct vec2 pos,
				 struct vec2 size, u32 flags)
{
	struct window *win = malloc(sizeof(*win));
	win->id = rand();
	win->name = name; // strdup?
	win->ctx.size = size;
	win->ctx.bpp = screen.bpp;
	win->ctx.pitch = size.x * bypp;
	win->ctx.bytes = win->ctx.pitch * win->ctx.size.y;
	if ((flags & WF_NO_FB) != 0) {
		win->ctx.fb = NULL;
	} else {
		assert(shalloc(win->ctx.bytes, (u32 *)&win->ctx.fb, &win->shid) == EOK);
	}
	win->client = client;
	win->flags = flags;
	win->pos = pos;
	win->pos_prev = pos;
	list_add(windows, win);
	return win;
}

static struct window *window_find(u32 id)
{
	struct node *iterator = windows->head;
	while (iterator) {
		struct window *win = iterator->data;
		if (win->id == id)
			return win;
		iterator = iterator->next;
	}
	return NULL;
}

static struct window *window_at(vec2 pos)
{
	struct window *ret = NULL;

	struct node *iterator = windows->head;
	while (iterator) {
		struct window *win = iterator->data;
		if (!(win->flags & (WF_NO_WINDOW | WF_NO_FOCUS)) && pos.x >= win->pos.x &&
		    pos.x <= win->pos.x + win->ctx.size.x && pos.y >= win->pos.y &&
		    pos.y <= win->pos.y + win->ctx.size.y)
			ret = win;
		iterator = iterator->next;
	}
	return ret;
}

static void window_redraw(struct window *win)
{
	// TODO: Only redraw difference of prev/curr (difficult with negative directions)
	vec2 pos1 = win->pos_prev;
	vec2 pos2 = vec2(pos1.x + win->ctx.size.x, pos1.y + win->ctx.size.y);

	rectangle_redraw(pos1, pos2, win);
	if (win != cursor) {
		window_redraw(cursor);
		buffer_flush();
	}
}

// TODO: Fix strange artifacts after destroying
static void window_destroy(struct window *win)
{
	//free(win->name);
	memset(win->ctx.fb, 0, win->ctx.bytes);
	rectangle_redraw(win->pos, vec2_add(win->pos, win->ctx.size), win);
	buffer_flush();
	list_remove(windows, list_first_data(windows, win));
	sys_free(win->ctx.fb);
	free(win);
}

/**
 * Event handlers
 */

static void handle_event_keyboard(struct event_keyboard *event)
{
	if (event->magic != KEYBOARD_MAGIC) {
		log("Keyboard magic doesn't match!\n");
		return;
	}

	if (event->scancode == KEY_LEFTSHIFT || event->scancode == KEY_RIGHTSHIFT)
		special_keys.shift ^= 1;
	else if (event->scancode == KEY_LEFTALT || event->scancode == KEY_RIGHTALT)
		special_keys.alt ^= 1;
	else if (event->scancode == KEY_LEFTCTRL || event->scancode == KEY_RIGHTCTRL)
		special_keys.ctrl ^= 1;

	if (event->scancode > KEYMAP_LENGTH)
		return;

	char ch;
	if (special_keys.shift)
		ch = keymap->shift_map[event->scancode];
	else if (special_keys.alt)
		ch = keymap->alt_map[event->scancode];
	else
		ch = keymap->map[event->scancode];

	(void)ch;
}

static void handle_event_mouse(struct event_mouse *event)
{
	if (event->magic != MOUSE_MAGIC) {
		log("Mouse magic doesn't match!\n");
		return;
	}

	cursor->pos_prev = mouse.pos;

	mouse.pos.x += event->diff_x;
	mouse.pos.y -= event->diff_y;

	// Fix x overflow
	if ((signed)mouse.pos.x < 0)
		mouse.pos.x = 0;
	else if (mouse.pos.x + cursor->ctx.size.x > (unsigned)screen.width - 1)
		mouse.pos.x = screen.width - cursor->ctx.size.x - 1;

	// Fix y overflow
	if ((signed)mouse.pos.y < 0)
		mouse.pos.y = 0;
	else if (mouse.pos.y + cursor->ctx.size.y > (unsigned)screen.height - 1)
		mouse.pos.y = screen.height - cursor->ctx.size.y - 1;

	cursor->pos = mouse.pos;

	struct window *win = window_at(mouse.pos);
	if (win && !(win->flags & WF_NO_FOCUS) && !event->but1 && !event->but2 && !event->but3)
		focused = win;

	if (focused && !(focused->flags & WF_NO_DRAG) && event->but1 && special_keys.alt) {
		focused->pos_prev = focused->pos;
		focused->pos = mouse.pos;
		window_redraw(focused);
		return;
	} else if (!vec2_eq(cursor->pos, cursor->pos_prev)) {
		window_redraw(cursor);
		buffer_flush();
	}

	if (!win)
		return;

	struct message_mouse msg = { 0 };
	msg.header.state = MSG_GO_ON;
	msg.pos = vec2_sub(mouse.pos, win->pos);
	msg_send(win->client.pid, GUI_MOUSE, &msg, sizeof(msg));
}

/**
 * Message handlers
 */

static void handle_message_new_window(struct message_new_window *msg)
{
	struct window *win = window_new((struct client){ .pid = msg->header.src }, "idk",
					vec2(500, 600), vec2(600, 400), 0);
	msg->ctx = win->ctx;
	msg->shid = win->shid;
	msg->id = win->id;

	if (msg->header.state == MSG_NEED_ANSWER)
		msg_send(msg->header.src, GUI_NEW_WINDOW | MSG_SUCCESS, msg, sizeof(*msg));
}

static void handle_message_redraw_window(struct message_redraw_window *msg)
{
	u32 id = msg->id;
	struct window *win = window_find(id);
	if (!win) {
		if (msg->header.state == MSG_NEED_ANSWER)
			msg_send(msg->header.src, GUI_REDRAW_WINDOW | MSG_FAILURE, NULL,
				 sizeof(msg->header));
		return;
	}

	window_redraw(win);

	if (msg->header.state == MSG_NEED_ANSWER)
		msg_send(msg->header.src, GUI_REDRAW_WINDOW | MSG_SUCCESS, msg,
			 sizeof(msg->header));
}

static void handle_message_destroy_window(struct message_destroy_window *msg)
{
	u32 id = msg->id;
	struct window *win = window_find(id);
	if (!win) {
		if (msg->header.state == MSG_NEED_ANSWER)
			msg_send(msg->header.src, GUI_DESTROY_WINDOW | MSG_FAILURE, NULL,
				 sizeof(msg->header));
		return;
	}

	window_destroy(win);

	if (msg->header.state == MSG_NEED_ANSWER)
		msg_send(msg->header.src, GUI_DESTROY_WINDOW | MSG_SUCCESS, msg,
			 sizeof(msg->header));
}

static void handle_message(void *msg)
{
	struct message_header *header = msg;

	switch (header->type) {
	case GUI_NEW_WINDOW:
		handle_message_new_window(msg);
		break;
	case GUI_REDRAW_WINDOW:
		handle_message_redraw_window(msg);
		break;
	case GUI_DESTROY_WINDOW:
		handle_message_destroy_window(msg);
		break;
	default:
		log("Message type %d not implemented!\n", header->type);
		msg_send(header->src, header->type | MSG_FAILURE, msg, sizeof(*header));
	}
}

static void handle_exit(void)
{
	if (keymap)
		free(keymap);
	if (windows)
		list_destroy(windows);
	if (screen.fb)
		memset(screen.fb, COLOR_RED, screen.height * screen.pitch);
}

/**
 * Main loop
 */

int main(int argc, char **argv)
{
	UNUSED(argc);
	UNUSED(argv);

	atexit(handle_exit);

	assert(ioctl("/dev/fb", IO_FB_GET, &screen) == 0);
	log("WM loaded: %dx%d\n", screen.width, screen.height);
	wm_client = (struct client){ .pid = getpid() };
	bypp = (screen.bpp >> 3);

	windows = list_new();
	keymap = keymap_parse("/res/keymaps/en.keymap");

	direct = window_new(wm_client, "direct", vec2(0, 0), vec2(screen.width, screen.height),
			    WF_NO_WINDOW | WF_NO_FB | WF_NO_DRAG | WF_NO_FOCUS | WF_NO_RESIZE);
	direct->ctx.fb = screen.fb;
	direct->flags ^= WF_NO_FB;
	root = window_new(wm_client, "root", vec2(0, 0), vec2(screen.width, screen.height),
			  WF_NO_WINDOW | WF_NO_DRAG | WF_NO_FOCUS | WF_NO_RESIZE);
	wallpaper =
		window_new(wm_client, "wallpaper", vec2(0, 0), vec2(screen.width, screen.height),
			   WF_NO_DRAG | WF_NO_FOCUS | WF_NO_RESIZE);
	// TODO: Fix strange cursor size segfault
	cursor = window_new(wm_client, "cursor", vec2(0, 0), vec2(32, 33),
			    WF_NO_WINDOW | WF_NO_DRAG | WF_NO_FOCUS | WF_NO_RESIZE);

	/* gfx_write(&direct->ctx, vec2(0, 0), FONT_32, COLOR_FG, "Loading Melvix..."); */
	gfx_load_wallpaper(&wallpaper->ctx, "/res/wall.png");
	gfx_load_wallpaper(&cursor->ctx, "/res/cursor.png");
	window_redraw(wallpaper);

	u8 msg[1024] = { 0 };
	struct event_keyboard event_keyboard = { 0 };
	struct event_mouse event_mouse = { 0 };
	const char *listeners[] = { "/dev/kbd", "/dev/mouse", "/proc/self/msg", NULL };
	while (1) {
		int poll_ret = 0;
		if ((poll_ret = poll(listeners)) >= 0) {
			if (poll_ret == 0) {
				if (read(listeners[poll_ret], &event_keyboard, 0,
					 sizeof(event_keyboard)) > 0) {
					handle_event_keyboard(&event_keyboard);
					continue;
				}
			} else if (poll_ret == 1) {
				if (read(listeners[poll_ret], &event_mouse, 0,
					 sizeof(event_mouse)) > 0) {
					handle_event_mouse(&event_mouse);
					continue;
				}
			} else if (poll_ret == 2) {
				if (msg_receive(msg, 1024) > 0) {
					handle_message(msg);
					continue;
				}
			}
		}
		panic("Poll/read error: %s\n", strerror(errno));
	}

	return 0;
}
