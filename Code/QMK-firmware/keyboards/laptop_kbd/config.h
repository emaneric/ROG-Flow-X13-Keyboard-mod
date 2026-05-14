#pragma once

/* Ghost key detection: suppress phantom keys that appear when 3 keys
 * form 3 corners of a row/col rectangle in a diode-less matrix. */
#define MATRIX_HAS_GHOST

/* MATRIX_IO_DELAY is the post-read delay (after unselecting a row).
 * The fix for FPC capacitive coupling is in keyboard.c, which overrides
 * matrix_output_select_delay() with a 1ms pre-read wait. */
