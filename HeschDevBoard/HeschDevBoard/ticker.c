#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <util/delay.h>

#include "ticker.h"
#include "Hardware Libs/st7565r.h"


font_t f;

unsigned char                    ticker_str[MAX_TICKER_LEN + 1];
uint_fast16_t                    ticker_col = 0;
unsigned char *                  ticker_ptr = ticker_str;

unsigned char matrix[DISPLAY_COLUMNS];

void delay_msec (uint_fast16_t delay)
{
	_delay_ms(500);
}

void show_display (void)
{
    int x;

    for (x = 0; x < DISPLAY_COLUMNS; x++)
    {
        glcdSetPageColumn (0, x);
        glcdSendData (matrix[x]);
    }
}

void set_pixel(uint_fast16_t line, uint_fast16_t col)
{
    matrix[col] |= (1 << line);
}

void clear_pixel (uint_fast16_t line, uint_fast16_t col)
{
    matrix[col] &= ~(1 << line);
}

void display_show_ticker_char (uint_fast16_t start_line, uint_fast16_t start_col, unsigned char ch, uint_fast16_t col_offset, font_t f)
{
    uint_fast16_t    line;
    uint_fast16_t    col;
    uint_fast16_t    offset;

	f = calcFontStart(ch,f,f.fontPtr);

    for (line = 0; line < TICKER_LINES; line++)
    {
        offset = col_offset + 1;

        for (col = 0; offset < TICKER_COLS + 1; col++, offset++)
        {
            if (start_line + line < DISPLAY_ROWS && start_col + col < DISPLAY_COLUMNS)
            {
                if (f.fontPtr[f.indexNum+line] & (1<<(TICKER_COLS - offset)))
                {
                    set_pixel (start_line + line, start_col + col);
                }
                else
                {
                    clear_pixel (start_line + line, start_col + col);
                }
            }
        }

        while (start_col + col < DISPLAY_COLUMNS)
        {
            clear_pixel (start_line + line, start_col + col);
            col++;
        }
    }
}

void display_ticker_with_offset (unsigned char * str, uint_fast16_t n)
{
    uint_fast16_t     start_line;
    uint_fast16_t     start_col;

    start_line = 0;
    start_col  = 0;

    display_show_ticker_char (start_line, start_col, *str, n, f);

    str++;
    start_col = TICKER_COLS + 1 - n;

    while (*str && start_col < DISPLAY_COLUMNS)
    {
        display_show_ticker_char (start_line, start_col, *str++, 0, f);
        start_col += TICKER_COLS + 1;                                       // +1 = gap between letters
    }

    while (start_col < DISPLAY_COLUMNS)
    {
        display_show_ticker_char (start_line, start_col, ' ', 0, f);
        start_col += TICKER_COLS + 1;                                       // +1 = gap between letters
    }
}

void display_ticker (void)
{
    display_ticker_with_offset (ticker_ptr, ticker_col);
    ticker_col++;

    if (ticker_col == TICKER_COLS + 1)
    {
        ticker_col = 0;
        ticker_ptr++;
    }
}

void display_set_ticker (const char * ticker)
{
    ticker_str[0] = ' ';
    ticker_str[1] = ' ';
    strncpy ((char *) ticker_str + 2, ticker, MAX_TICKER_LEN - 2);
    ticker_col = 0;
    ticker_ptr = ticker_str;

    memset (matrix, 0, DISPLAY_COLUMNS);

    while (*ticker_ptr)
    {
        display_ticker ();
        show_display ();
        delay_msec (TICKER_FRAME_TIME);
    }
}
