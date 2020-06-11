#include <drivers/video/font.h>
#include <drivers/video/video.h>
#include <fs/filedescriptor.h>
#include <libk/string.h>
#include <mm/heap.h>
#include <proc/tasking.h>
#include <kernel/tty_dev.h>

/**
 * A tty device is going to be a file that can be written to
 * the function that the vfs will hold will be tty_write
 *
 * @note: I have no clue if this system is actually posix compliant and I even
 * think it isn't but I'm not certain and it looks like good code so why remove
 * what works
 */

unsigned int g_current_tty      = 0;
tty_ctrl_t * tty_control_struct = 0;

struct escape_seq
{
	int        iter;
	int        curcol;
	int        currow;
	int        startc;
	int        startr;
	tty_dev_t *ttydev;
};

void init_tty_stdin(void);

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
int init_tty_devices()
{
	// calculate the amount of columns and rows this system will have
	int cols, rows;
	if (get_video_mode() != VIDEO_MODE_TERM)
	{
		/**
		 * We are not in terminal mode so we should calculate the amount of
		 * columns and rows
		 */

		cols = video_get_screen_width() / FONTWIDTH;
		rows = video_get_screen_height() / FONTHEIGHT;
	}
	else
	{
		/**
		 * If our video mode is terminal mode, then we aren't in vesa mode
		 * and our width and height will already be a column and row count
		 */
		cols = video_get_screen_width();
		rows = video_get_screen_height();
	}

	// calculate the tty device buffer size
	int tty_buf_size = cols * rows * (sizeof(colorbit_t) + sizeof(char));

	// allocate the tty control structure
	tty_control_struct = kmalloc(sizeof(tty_ctrl_t));

	// check if memory allocation succeeded
	if (tty_control_struct == 0)
	{
		return -1;
	}

	/* set the necessary values */
	tty_control_struct->col_max       = cols;
	tty_control_struct->row_max       = rows;
	tty_control_struct->colorbit_size = sizeof(colorbit_t);
	tty_control_struct->char_size     = sizeof(char);
	tty_control_struct->default_color = TTY_WHITE | (TTY_BLACK << 4);
	tty_control_struct->color         = tty_control_struct->default_color;

	tty_control_struct->tty_devices =
		kmalloc(sizeof(tty_dev_t) * TTY_DEVICE_AMOUNT);

	/* check if memory allocation succeeded */
	if (tty_control_struct->tty_devices == 0)
		return -1;

	/* initialize each tty device */
	for (size_t i = 0; i < TTY_DEVICE_AMOUNT; i++)
	{
		tty_control_struct->tty_devices[i].buffer   = kmalloc(tty_buf_size);
		tty_control_struct->tty_devices[i].id       = i;
		tty_control_struct->tty_devices[i].c_col    = 0;
		tty_control_struct->tty_devices[i].c_row    = 0;
		tty_control_struct->tty_devices[i].disabled = 0;
	}

	/* Kernel will get tty 0 for now, i haven't completely tought this tty
	 * system through */
	get_current_task()->tty = 0;

	char *buf = tty_control_struct->tty_devices[0].buffer;
	for (int i = 0; i < tty_buf_size; i += 2)
	{
		buf[i + 1] = tty_control_struct->color;
	}

	// return 0 on success
	return 0;
}

void tty_disable()
{
	tty_dev_t *tty = tty_get_device(get_current_task()->tty);
	tty->disabled  = 1;
}

void tty_enable()
{
	tty_dev_t *tty = tty_get_device(get_current_task()->tty);
	tty->disabled  = 0;
}

void tty_update_display(tty_dev_t *tty_dev, int startcol, int startrow,
                        int endcol, int endrow)
{
	/**
	 * The reason this function has a startlocation and endlocation argument is
	 * for optimization see these locations indicate locations in the buffer
	 * from where it needs to be updated so that we do not have to do the whole
	 * buffer every time
	 */

	/**
	 * First calculate the start and end locations in the buffer
	 */
	unsigned int startloc =
		(tty_control_struct->col_max * startrow + startcol)
		* (tty_control_struct->char_size + tty_control_struct->colorbit_size);
	unsigned int endloc =
		(tty_control_struct->col_max * endrow + endcol)
		* (tty_control_struct->char_size + tty_control_struct->colorbit_size);

	/**
	 * Now loop over the bytes in the chars in the buffer and update them to the
	 * screen
	 */

	for (size_t i = startloc; i < endloc;
	     i +=
	     (tty_control_struct->char_size + tty_control_struct->colorbit_size))
	{
		// draw character at correct location with correct color
		int color = tty_dev->buffer[i + 1] & 0xF;
		int bg    = tty_dev->buffer[i + 1] >> 4;

		// xloc and yloc are actually the terminal column and row
		int xloc = (i
		            / (tty_control_struct->char_size
		               + tty_control_struct->colorbit_size))
				   % tty_control_struct->col_max;
		int yloc = (i
		            / (tty_control_struct->char_size
		               + tty_control_struct->colorbit_size))
				   / tty_control_struct->col_max;

		/**
		 * If our screen is not in terminal mode, we should calculate the actual
		 * pixel location of the text we would also need to recalculate the
		 * color to full rgb instead of vga 8bit color
		 */
		if (get_video_mode() != VIDEO_MODE_TERM)
		{
			// recalculate the color
			color = video_vga_to_rgb(color);
			bg    = video_vga_to_rgb(bg);

			xloc *= video_get_screen_width() / tty_control_struct->col_max;
			yloc *= video_get_screen_height() / tty_control_struct->row_max;

			if (tty_dev->buffer[i] > ' ' && tty_dev->buffer[i] <= '~')
			{
				// video_clear_cell(xloc, yloc);
				video_draw_char(' ', xloc, yloc, color, bg);
			}
		}

		video_draw_char(tty_dev->buffer[i], xloc, yloc, color, bg);
	}

	video_update_cursor(tty_dev->c_col, tty_dev->c_row);
}

/**
 * @brief      A function that will scroll all the characters and colors of the
 * tty buffer 1 row up
 *
 * @param      tty_dev  The tty dev
 */
static void _tty_scroll(tty_dev_t *tty_dev)
{
	// calculate the row size here for efficiency
	unsigned int rowsize =
		tty_control_struct->col_max
		* (tty_control_struct->colorbit_size + tty_control_struct->char_size);

	// loop over all the rows starting at the second one (index = 1), and copy
	// them all one up
	for (size_t i = 1; i < tty_control_struct->row_max; i++)
	{
		void *line2 = (void *) &tty_dev->buffer[i * rowsize];
		void *line1 = (void *) &tty_dev->buffer[(i - 1) * rowsize];
		memcpy(line1, line2, rowsize);
	}

	// now empty the next line
	// @todo: this is wrong !!! we should fill the line with ' ' characters
	// because now the bg color of the last row will be completely wrong
	memset(&tty_dev->buffer[(tty_control_struct->row_max - 1) * rowsize], 0,
	       rowsize);
	video_clear_screen();
	// tty_update_display(tty_dev, 0, 0, tty_control_struct->col_max - 1,
	// tty_control_struct->row_max - 1);
}

static unsigned int parse_dec_uint(char **str)
{
	int num = 0;
	while (**str)
	{
		if (**str < '0' || **str > '9')
			return num;

		num *= 10;
		num += **str - '0';
		*str += 1;
	}

	return num;
}

static int escape_to_tty_color(int escapecolor)
{
	if (tty_control_struct->bold && escapecolor >= 30 && escapecolor <= 37)
	{
		tty_control_struct->bold = 0;
		int color                = escape_to_tty_color(escapecolor + 60);
		tty_control_struct->bold = 1;

		return color;
	}

	switch (escapecolor)
	{
	case 30:
		return TTY_BLACK;
	case 31:
		return TTY_RED;
	case 32:
		return TTY_GREEN;
	case 33:
		return TTY_ORANGE;
	case 34:
		return TTY_BLUE;
	case 35:
		return TTY_PURPLE;
	case 36:
		return TTY_CYAN;
	case 37:
		return TTY_GRAY;

	case 90: /* Bright black ? lol */
		return TTY_DARK_GRAY;
	case 91: /* Bright red */
		return TTY_LIGHT_RED;
	case 92: /* Bright green */
		return TTY_LIGHT_GREEN;
	case 93: /* Bright yellow */
		return TTY_YELLOW;
	case 94: /* Bright blue */
		return TTY_LIGHT_BLUE;
	case 95: /* Bright magenta */
		return TTY_PINK;
	case 96: /* Bright cyan */
		return TTY_LIGHT_CYAN;
	case 97: /* Bright white ? again what */
		return TTY_WHITE;
	case 0:
		tty_control_struct->bold = 0;
		return TTY_WHITE;

	case 1:
		tty_control_struct->bold = 1;
		int color                = tty_control_struct->color;
		if (color <= TTY_GRAY && color >= TTY_BLACK)
			tty_control_struct->color += TTY_DARK_GRAY;

		return -1;
	}

	return -1;
}

static void parse_escape_control_sequence(struct escape_seq *es, char *seq)
{
	unsigned int param1;
	unsigned int param2;
	unsigned int start_seq = (unsigned int) seq;

	switch (*seq)
	{
	case 'H': /* Cursor home */
		es->curcol = 0;
		es->currow = 0;
		es->startc = 0;
		es->startr = 0;
		es->iter++;
		return;

	case 'J':
		es->iter++;
		tty_clear_buf(es->ttydev);
		return;

	case 'm':
		es->iter++;
		tty_set_color(tty_control_struct->default_color, -1);
		return;

	default:
		param1 = parse_dec_uint(&seq);

		if (*seq == 'm')
		{
			/* Set color */
			tty_set_color(escape_to_tty_color(param1), -1);
			break;
		}
		else if (*seq == 'A')
		{
			if ((int) (es->currow - param1) < 0)
				param1 = param1 - es->currow;

			es->currow -= param1;
			break;
		}

		else if (*seq == 'C')
		{
			if (es->curcol + param1 > tty_control_struct->col_max - 1)
				param1 = tty_control_struct->col_max - 1 - param1;

			es->curcol += param1;
			break;
		}

		/* Unknown escape sequence */
		if (*seq != ';')
			break;

		seq++;
		param2 = parse_dec_uint(&seq);

		if (*seq == 'H' || *seq == 'f')
		{
			es->curcol = param2 - 1;
			es->currow = param1 - 1;
			if (es->curcol < es->startc)
				es->startc = es->curcol;

			if (es->currow < es->startr)
				es->startr = es->currow;
		}
		else if (*seq == 'm')
		{
			tty_set_color(escape_to_tty_color(param1), -1);
			tty_set_color(escape_to_tty_color(param2), -1);
		}
	}
	es->iter += (unsigned int) seq - start_seq + 1;
}

static void parse_escape_sequence(struct escape_seq *es, char *seq)
{
	/* See: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797 */
	switch (*seq)
	{
	case '[': /* Control sequence introducer */
		es->iter++;
		parse_escape_control_sequence(es, seq + 1);
	}
}

static void tty_clear_cell(tty_dev_t *tty_dev, int col, int row)
{
	int l              = (row * tty_control_struct->col_max + col) * 2;
	tty_dev->buffer[l] = ' ';
	tty_update_display(tty_dev, col, row, col + 1, row);
}

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
                  size_t bytes_to_write, int col, int row)
{
	/**
	 * Just checks if the tty device we got is not a null pointer
	 */
	if (tty_dev == 0)
		return -1;

	if (tty_dev->disabled)
		return 0;

	/**
	 * If the column or row specified equals -1, we should write to the cursor
	 * location (and update the cursor location too)
	 */

	unsigned int cur_col, cur_row;
	if ((col == -1) | (row == -1))
	{
		cur_col = tty_dev->c_col;
		cur_row = tty_dev->c_row;
	}
	else
	{
		cur_col = col;
		cur_row = row;
	}
	/**
	 * These are for calculating from where to update the buffer
	 */
	unsigned int startc = cur_col;
	unsigned int startr = cur_row;

	/**
	 * Now we write every byte to the buffer (with appropriate color!)
	 * interpreting the color is for the tty to video function
	 */

	size_t i;
	int    dofullupdate =
		0; /* A variable for checking if we need to update the whole screen */

	for (i = 0; i < bytes_to_write; i++)
	{
		/* So we loop over every byte this means we need to filter out escape
		 * characters here */
		char character = text_to_write[i];
		if (character == '\n' || character == '\r')
		{
			/* newline / carriage return */
			cur_col = 0;
			cur_row++;
		}
		else if (character == '\t')
		{
			/* Horizontal tab */
			cur_col += 4;
		}
		else if (character == '\033')
		{
			/* This will be an ansi escape sequence */
			struct escape_seq es;
			es.iter   = i;
			es.startc = startc;
			es.startr = startr;
			es.curcol = cur_col;
			es.currow = cur_row;
			es.ttydev = tty_dev;

			parse_escape_sequence(&es, (char *) text_to_write + i + 1);

			i       = es.iter;
			startc  = es.startc;
			startr  = es.startr;
			cur_col = es.curcol;
			cur_row = es.currow;
		}
		else if (character == 0x8)
		{
			if (cur_col)
			{
				tty_clear_cell(tty_dev, cur_col - 1, cur_row);
				cur_col--;
			}
		}
		else
		{
			/* Regular character */

			/* Just cleaner doing the calculation here */
			int l = (cur_row * tty_control_struct->col_max + cur_col)
					* (tty_control_struct->colorbit_size
			           + tty_control_struct->char_size);

			/* Writing the character to the buffer with the appropriate color
			 * after it */
			tty_dev->buffer[l] = character;
			tty_dev->buffer[l + sizeof(char)] =
				(colorbit_t) tty_control_struct->color;

			/* adding to the column because we added 1 character */
			cur_col++;
		}

		/* Now just handle scrolling and wrapping */
		if (cur_col >= tty_control_struct->col_max)
		{
			/* Line end reached we need to wrap to next line*/
			cur_col = 0;
			cur_row++;
		}

		if (cur_row >= tty_control_struct->row_max)
		{
			/* End of screen reached we need to scroll down */
			_tty_scroll(tty_dev);
			dofullupdate = 1;
			cur_col      = 0;
			cur_row--;
		}
	}

	/* If the column was not specified we do not need to change the cursor
	 * location */
	if ((col == -1) | (row == -1))
	{
		tty_dev->c_col = cur_col;
		tty_dev->c_row = cur_row;
	}

	/* Updating the display to show the actual characters */
	if (dofullupdate)
	{
		startc  = 0;
		startr  = 0;
		cur_col = tty_control_struct->col_max - 1;
		cur_row = tty_control_struct->row_max - 1;
	}

	tty_update_display(tty_dev, startc, startr, cur_col, cur_row);

	/**
	 * this is actually pretty dumb but I don't see how this function could fail
	 * in any way except the buffer not being initialized which would give a
	 * segfault
	 */
	return i + 1;
}

/**
 * @brief      This function will return a tty device pointer by tty device
 * identifier
 *
 * @param[in]  tty_id  The tty identifier
 *
 * @return     The tty device pointer
 */
tty_dev_t *tty_get_device(unsigned int tty_id)
{
	return &tty_control_struct->tty_devices[tty_id];
}

/**
 * @brief      Sets the tty colorcode for future text
 *
 * @param[in]  color  The color
 */
void tty_set_color(int fg, int bg)
{
	if (fg != -1)
	{
		tty_control_struct->color &= 0xF0;
		tty_control_struct->color |= fg;
	}

	if (bg != -1)
	{
		tty_control_struct->color &= 0xF;
		tty_control_struct->color |= bg << 4;
	}
}

void tty_reset_color()
{
	tty_control_struct->color = tty_control_struct->default_color;
}

/**
 * @brief      Sets the tty cursor column location
 *
 * @param      tty_dev  The tty dev
 * @param[in]  newcol   The new column location
 */
void tty_set_cursor_col(tty_dev_t *tty_dev, int newcol)
{
	tty_dev->c_col = newcol;
}

/**
 * @brief      Sets the tty cursor row location
 *
 * @param      tty_dev  The tty dev
 * @param[in]  newrow   The new row location
 */
void tty_set_cursor_row(tty_dev_t *tty_dev, int newrow)
{
	tty_dev->c_row = newrow;
}

/**
 * @brief      Clears the screen and resets terminal
 *
 * @param      tty_dev  The tty dev
 */
void tty_clear_buf(tty_dev_t *tty_dev)
{
	/* First overwrite the buffer with ' ' character */
	for (size_t i = 0; i < tty_control_struct->col_max; i++)
	{
		for (size_t j = 0; j < tty_control_struct->row_max; j++)
		{
			/* The - 1 at the end is because indexes start at 0 */
			int l                  = (j * tty_control_struct->col_max + i) * 2;
			tty_dev->buffer[l]     = ' ';
			tty_dev->buffer[l + 1] = tty_control_struct->color;
		}
	}

	/* Now reset cursor */
	tty_dev->c_col = 0;
	tty_dev->c_row = 0;

	/* Finally update the display */
	tty_update_display(tty_dev, 0, 0, tty_control_struct->col_max - 1,
	                   tty_control_struct->row_max - 1);
}
