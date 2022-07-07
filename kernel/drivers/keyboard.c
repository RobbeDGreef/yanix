#include <debug.h>
#include <drivers/keyboard.h>
#include <fs/vfs.h>
#include <libk/string.h>
#include <mm/heap.h>
#include <proc/tasking.h>
#include <drivers/eventdev.h>
#include <yanix/input.h>

char *keymap_normal = NULL;
char *keymap_shift  = NULL;
char *keymap_altgr  = NULL;

struct key_mod g_modifier;
int            eventfd = 0;

#define LSHIFT_D 0x2a
#define LSHIFT_U 0xaa

#define CAPS_LCK 0x3a

#define RSHIFT_D 0x36
#define RSHIFT_U 0xb6

#define ALTGR_D 56
#define ALTGR_U 184

#define RETURN_D 0x1C
#define LCTRL_D  0x1D
#define LCTRL_U  0x9D

#define BACKSPACE_D 0xE
#define BACKSPACE_U 0xBE

static int modifiers(int scancode)
{
	switch (scancode)
	{
	case LSHIFT_U:
	case RSHIFT_U:
		g_modifier.shift = 0;
		return 0;

	case LSHIFT_D:
	case RSHIFT_D:
		g_modifier.shift = 1;
		return 0;

	case CAPS_LCK:
		g_modifier.shift = g_modifier.shift ? 0 : 1;
		return 0;

	case ALTGR_D:
		g_modifier.altgr = 1;
		return 0;

	case ALTGR_U:
		g_modifier.altgr = 0;
		return 0;

	case LCTRL_U:
		g_modifier.ctrl = 0;
		return 0;

	case LCTRL_D:
		g_modifier.ctrl = 1;
		return 0;

	case BACKSPACE_D:
		return 0x8;

	case RETURN_D:
		return '\n';
	}

	return -1;
}

static int getkey(int scancode)
{
	int c;
	if ((c = modifiers(scancode)) >= 0)
		return c;

	if (scancode > 128)
		return 0;

	if (g_modifier.shift)
		return keymap_shift[scancode];
	if (g_modifier.altgr)
		return keymap_altgr[scancode];
	if (keymap_normal)
		return keymap_normal[scancode];

	return 0;
}

static int combination(int c)
{
	if (g_modifier.ctrl && c == 'c')
	{
		send_sig(SIGINT);
		return 1;
	}

	return 0;
}

int send_scancode(int scancode)
{
	if (keymap_normal == NULL)
		return 0;

	char c = getkey(scancode);

	struct yinp_kb_pkt pkt = INP_KB_INIT(scancode, KEYMODE_PRESSED, c);
	vfs_write_fd(eventfd, &pkt, sizeof(pkt));

	int interuptchar = '^';
	if (combination(c))
		vfs_write_fd(0, &interuptchar, 1);

	if (c)
		vfs_write_fd(0, &c, 1);

	return 0;
}

static void set_keymap(char *map, char *buf, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (buf[i] == '\n')
			map[i] = i;
		else
		{
			map[i] = buf[i];
			buf++;
		}
	}
}

int init_keyboard()
{
	keymap_normal = kmalloc(KEYMAP_SIZE);
	keymap_shift  = kmalloc(KEYMAP_SIZE);
	keymap_altgr  = kmalloc(KEYMAP_SIZE);

	memset(&g_modifier, 0, sizeof(struct key_mod));

	char *       keymap = kmalloc(32);
	struct file *fp     = vfs_open(KEYMAP_FILE, 0, 0);
	vfs_read(fp, keymap, 32);
	vfs_close(fp);

	char *file = kmalloc(256);
	char *buf  = kmalloc(KEYMAP_FILE_SIZE);

	strcpy_s(file, KEYMAP_FILE_PREFIX, 256);
	strcat(file, keymap);
	strcat(file, "-normal");
	fp = vfs_open(file, 0, 0);
	vfs_read(fp, buf, KEYMAP_FILE_SIZE);
	set_keymap(keymap_normal, buf, KEYMAP_SIZE);
	vfs_close(fp);

	strcpy_s(file, KEYMAP_FILE_PREFIX, 256);
	strcat(file, keymap);
	strcat(file, "-shift");
	fp = vfs_open(file, 0, 0);
	vfs_read(fp, buf, KEYMAP_FILE_SIZE);
	set_keymap(keymap_shift, buf, KEYMAP_SIZE);
	vfs_close(fp);

	strcpy_s(file, KEYMAP_FILE_PREFIX, 256);
	strcat(file, keymap);
	strcat(file, "-altgr");
	fp = vfs_open(file, 0, 0);
	vfs_read(fp, buf, KEYMAP_FILE_SIZE);
	set_keymap(keymap_altgr, buf, KEYMAP_SIZE);
	vfs_close(fp);

	kfree(keymap);
	kfree(file);
	kfree(buf);

	eventfd = eventdev_create();

	return 0;
}