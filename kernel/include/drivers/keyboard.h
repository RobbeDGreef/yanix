#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#define KEYMAP_FILE        "/etc/default/keymap"
#define KEYMAP_FILE_PREFIX "/etc/keyboard/"
#define KEYMAP_SIZE        128
#define KEYMAP_FILE_SIZE   256

struct key_mod
{
	int ctrl;
	int shift;
	int alt;
	int altgr;
};

int send_scancode(int scancode);
int init_keyboard();

#endif /* _KEYBOARD_H */