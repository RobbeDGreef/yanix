#ifndef _TTY_DEV_H
#define _TTY_DEV_H

#include <stdint.h>
#include <sys/types.h>

#define TTY_DEVICE_AMOUNT 3

// @todo: this variable should actually be an array to support multiple monitors
/* The currently selected tty */
extern unsigned int g_current_tty;

typedef uint8_t colorbit_t;

#define TTY_BLACK       0x00
#define TTY_BLUE        0x01
#define TTY_GREEN       0x02
#define TTY_CYAN        0x03
#define TTY_RED         0x04
#define TTY_PURPLE      0x05
#define TTY_ORANGE      0x06
#define TTY_GRAY        0x07
#define TTY_DARK_GRAY   0x08
#define TTY_LIGHT_BLUE  0x09
#define TTY_LIGHT_GREEN 0x0a
#define TTY_LIGHT_CYAN  0x0b
#define TTY_LIGHT_RED   0x0c
#define TTY_PINK        0x0d
#define TTY_YELLOW      0x0e
#define TTY_WHITE       0x0f

/**
 * The tty device structure
 */
typedef struct tty_dev_s
{
	char *       buffer;
	unsigned int id;
	unsigned int c_col;
	unsigned int c_row;
	int          disabled;

} tty_dev_t;

/**
 * The control structure for the tty devices
 */
typedef struct tty_ctrl_s
{
	unsigned int col_max;
	unsigned int row_max;
	unsigned int colorbit_size;
	unsigned int char_size; // just sizeof(char) but it's cleaner this way
	unsigned int color;
	unsigned int default_color;
	int          bold;
	tty_dev_t *  tty_devices; // pointer to array of tty devices
} tty_ctrl_t;

/* Functions */

/**
 * @brief      This function will initialize the tty devices and create all the
 * necessary buffers
 *
 * @param[in]  cols            The maximum amount columns of text
 * @param[in]  rows            The maximum amount rows of text
 * @param[in]  tty_dev_amount  The amount of tty devices
 *
 * @return     Returns 0 on success
 */
int init_tty_devices();

/**
 * @brief      This function will write bytes to a tty device buffer
 *
 * @param      tty_dev         The tty dev
 * @param      text_to_write   The text to write
 * @param[in]  bytes_to_write  The bytes to write
 * @param[in]  col             The column location to write from
 * @param[in]  row             The row location to write from
 *
 * @return     The amount of bytes written
 */
ssize_t tty_write(tty_dev_t *tty_dev, const char *text_to_write,
                  size_t bytes_to_write, int col, int row);

/**
 * @brief      This function will return a tty device pointer by tty device
 * identifier
 *
 * @param[in]  tty_id  The tty identifier
 *
 * @return     The tty device pointer
 */
tty_dev_t *tty_get_device(unsigned int tty_id);

void tty_set_color(int fg, int bg);
void tty_reset_color();

/**
 * @brief      Sets the tty cursor column location
 *
 * @param      tty_dev  The tty dev
 * @param[in]  newcol   The new column location
 */
void tty_set_cursor_col(tty_dev_t *tty_dev, int newcol);

/**
 * @brief      Sets the tty cursor row location
 *
 * @param      tty_dev  The tty dev
 * @param[in]  newrow   The new row location
 */
void tty_set_cursor_row(tty_dev_t *tty_dev, int newrow);

/**
 * @brief      Clears the screen and resets terminal
 *
 * @param      tty_dev  The tty dev
 */
void tty_clear_buf(tty_dev_t *tty_dev);

void tty_enable();
void tty_disable();

#endif