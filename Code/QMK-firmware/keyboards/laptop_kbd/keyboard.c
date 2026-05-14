#include QMK_KEYBOARD_H

/* The FPC cable has significant capacitive coupling between adjacent Port D
 * traces (cols 12-15 = PD0-PD3). The default pre-read delay (waitInputPinDelay)
 * is only ~250ns — far too short for the coupling transient to clear.
 * This overrides matrix_output_select_delay to use 1ms, matching the
 * HAL_Delay(1) in the working STM32 code, which is called after driving a
 * row LOW and before reading columns. */
void matrix_output_select_delay(void) {
    wait_us(1000);
}
