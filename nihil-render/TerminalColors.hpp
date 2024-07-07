#pragma once
#ifdef USE_COLORS
	#define RESET       "\x1B[0m"
	// ANSI escape codes for text color
	#define BLACK       "\x1B[30m"
	#define RED         "\x1B[31m"
	#define GREEN       "\x1B[32m"
	#define YELLOW      "\x1B[33m"
	#define BLUE        "\x1B[34m"
	#define MAGENTA     "\x1B[35m"
	#define CYAN        "\x1B[36m"
	#define WHITE       "\x1B[37m"

	// ANSI escape codes for background color
	#define BG_BLACK    "\x1B[40m"
	#define BG_RED      "\x1B[41m"
	#define BG_GREEN    "\x1B[42m"
	#define BG_YELLOW   "\x1B[43m"
	#define BG_BLUE     "\x1B[44m"
	#define BG_MAGENTA  "\x1B[45m"
	#define BG_CYAN     "\x1B[46m"
	#define BG_WHITE    "\x1B[47m"

	// ANSI escape codes for text style
	#define BOLD        "\x1B[1m"
	#define UNDERLINE   "\x1B[4m"
	#define INVERT      "\x1B[7m"
#endif
#ifndef USE_COLORS
	#define RESET       ""
	// ANSI escape codes for text color
	#define BLACK       ""
	#define RED         ""
	#define GREEN       ""
	#define YELLOW      ""
	#define BLUE        ""
	#define MAGENTA     ""
	#define CYAN        ""
	#define WHITE       ""

	// ANSI escape codes for background color
	#define BG_BLACK    ""
	#define BG_RED      ""
	#define BG_GREEN    ""
	#define BG_YELLOW   ""
	#define BG_BLUE     ""
	#define BG_MAGENTA  ""
	#define BG_CYAN     ""
	#define BG_WHITE    ""

	// ANSI escape codes for text style
	#define BOLD        ""
	#define UNDERLINE   ""
	#define INVERT      ""
#endif
