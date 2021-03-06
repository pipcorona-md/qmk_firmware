#ifdef RGB_MATRIX_ENABLE
#include "carina.h"

#include "led_matrix.h"
#include "rgb_matrix.h"
#include "config_led.h"

led_config_t g_led_config = {
  {
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13 },
    { 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27 },
    { 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, NO_LED, 40 },
    { 41, NO_LED, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, NO_LED, 52 },
    { 53, 54, 55, NO_LED, NO_LED, NO_LED, 56, NO_LED, NO_LED, NO_LED, 57, 58, 59, 60 }
  },
  {
    {   8, 56 },
    {  22, 56 },
    {  35, 56 },
    {  49, 56 },
    {  63, 56 },
    {  77, 56 },
    {  91, 56 },
    { 105, 56 },
    { 118, 56 },
    { 132, 56 },
    { 146, 56 },
    { 160, 56 },
    { 174, 56 },
    { 195, 56 },

    {  11, 44 },
    {  28, 44 },
    {  42, 44 },
    {  56, 44 },
    {  70, 44 },
    {  84, 44 },
    {  98, 44 },
    { 112, 44 },
    { 125, 44 },
    { 139, 44 },
    { 153, 44 },
    { 167, 44 },
    { 181, 44 },
    { 198, 44 },

    {  13, 32 },
    {  32, 32 },
    {  46, 32 },
    {  60, 32 },
    {  73, 32 },
    {  87, 32 },
    { 101, 32 },
    { 115, 32 },
    { 129, 32 },
    { 143, 32 },
    { 156, 32 },
    { 170, 32 },
    // ___
    { 193, 32 },

    {  16, 19 },
    {  39, 19 },
    {  53, 19 },
    {  67, 19 },
    {  80, 19 },
    {  94, 19 },
    { 108, 19 },
    { 122, 19 },
    { 136, 19 },
    { 150, 19 },
    { 163, 19 },
    // ___
    { 182, 19 },

    {   9,  7 },
    {  27,  7 },
    {  44,  7 },
    // ___
    // ___
    // ___
    {  96,  7 },
    // ___
    // ___
    // ___
    { 148,  7 },
    { 165,  7 },
    { 188,  7 },
    { 201,  7 },

    // Underglow
    {  15, 49 },
    {  35, 49 },
    {  53, 49 },
    {  70, 49 },
    {  88, 49 },
    { 106, 49 },
    { 124, 49 },
    { 142, 49 },
    { 161, 49 },
    { 179, 49 },
    { 197, 49 },
    {  15, 12 },
    {  35, 12 },
    {  53, 12 },
    {  70, 12 },
    {  88, 12 },
    { 106, 12 },
    { 124, 12 },
    { 142, 12 },
    { 161, 12 },
    { 176, 12 },
    { 194, 12 },
    {  15, 37 },
    {  15, 25 },
    { 197, 37 },
    { 197, 25 }
  },
  {
    1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,    1,
    4,    4, 4, 4, 4, 4, 4, 4, 4, 4, 4,    4,
    1, 1, 1,          4,          1, 1, 1, 1,

    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2
  }
};

#ifdef USB_LED_INDICATOR_ENABLE
void rgb_matrix_indicators_kb(void)
{
  led_matrix_indicators();
}
#endif // USB_LED_INDICATOR_ENABLE

#endif
