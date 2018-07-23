#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "Hardware Libs/st7565r.h"

#define DISPLAY_ROWS                    64                  // Number of lines of display, CHANGE HERE!
#define DISPLAY_COLUMNS                128                  // Number of columns of display, CHANGE HERE!
#define MAX_TICKER_LEN                  64                  // Maximum length of ticker string, CHANGE HERE!
#define TICKER_FRAME_TIME              100                  // Time of ticker frame, CHANGE HERE!

#define TICKER_LINES                    7
#define TICKER_COLS                     5

extern font_t f;

void delay_msec (uint_fast16_t delay);

void show_display (void);

void set_pixel (uint_fast16_t line, uint_fast16_t col);

void clear_pixel (uint_fast16_t line, uint_fast16_t col);

void display_show_ticker_char (uint_fast16_t start_line, uint_fast16_t start_col, unsigned char ch, uint_fast16_t col_offset, font_t f);

void display_ticker_with_offset (unsigned char * str, uint_fast16_t n);

void display_ticker (void);

void display_set_ticker (const char * ticker);

