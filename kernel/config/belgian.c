#include <config/belgian.h>
const char NormalKeyList[] = {
	0x0, 0x1, '&', 0x3,  '"',  '\'', '(',  0x7,  0x8,  '!', 0xa, 0xb,
	')', '-', 0xe, '\t', 'a',  'z',  'e',  'r',  't',  'y', 'u', 'i',
	'o', 'p', '^', '$',  '\n', 0x1d, 'q',  's',  'd',  'f', 'g', 'h',
	'j', 'k', 'l', 'm',  0x28, 0x29, 0x2a, 0x2b, 'w',  'x', 'c', 'v',
	'b', 'n', ',', ';',  ':',  '=',  0x36, '*',  0x38, ' ', 0x3a};
const char ShiftKeyList[] = {
	0x0, 0x1, '1', '2', '3',  '4',  '5',  '6',  '7',  '8', '9', 0xb,
	0xc, '_', 0xe, 0xf, 'A',  'Z',  'E',  'R',  'T',  'Y', 'U', 'I',
	'O', 'P', '"', '*', 0x1c, 0x1d, 'Q',  'S',  'D',  'F', 'G', 'H',
	'J', 'K', 'L', 'M', '%',  0x29, 0x2a, 0x2b, 'W',  'X', 'C', 'V',
	'B', 'N', '?', '.', '/',  '+',  0x36, '*',  0x38, ' ', 0x3a};
const char AltGrKeyList[] = {
	0x0,  0x1,  '|',  '@',  '#',  0x5,  0x6,  '^',  '{',  '[',  '{',  '}',
	'\\', 0xd,  0xe,  0xf,  '@',  0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, '[',  ']',  0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
	0x24, 0x25, 0x26, 0x27, '\'', 0x29, 0x2a, '`',  0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, '~',  0x36, '*',  0x38, ' ',  0x3a};
const char *KeyNames[] = {
	"Error",   "Escape",      "&",          "é",
	"\"",      "'",           "(",          "§",
	"è",       "!",           "ç",          "à",
	")",       "-",           "Backspace",  "Tab",
	"a",       "z",           "e",          "r",
	"t",       "y",           "u",          "i",
	"o",       "p",           "^",          "$",
	"Return",  "LeftControl", "q",          "s",
	"d",       "f",           "g",          "h",
	"j",       "k",           "l",          "m",
	"ù",       "SuperScript", "LeftShift",  "µ",
	"w",       "x",           "c",          "v",
	"b",       "n",           ",",          ";",
	":",       "=",           "RightShift", "Numpad-Multiply",
	"LeftAlt", "Spacebar",    "CapsLock"};
