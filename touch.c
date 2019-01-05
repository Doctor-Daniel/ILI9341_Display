/*
 * touch.c
 *
 *  Author: kostuch@skeletondevices.com
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include "touch.h"
#include "ILI9341.h"
#include "SPI.h"

static inline void XPT2046_activate(void);
static inline void XPT2046_deactivate(void);
static inline void XPT2046_wr_cmd(uint8_t tx);
static inline uint8_t XPT2046_rd_data(uint8_t tx);
static inline int16_t XPT2046_rd16(uint8_t control);
static inline uint8_t XPT2046_rd8(uint8_t control);

cal_t ee_touch_cal EEMEM;

// Calibration points on the (landscape) screen
// ------------------------
// | 1                    |
// |                      |
// |                    3 |
// |                      |
// |          2           |
// ------------------------
point_t EEMEM ee_cal_points[3] =											// Three calibration points
{
    {20, 20},
    {300, 120},
    {160, 220}
};

void XPT2046_init_io(void)
{
    // This is already done in TFT section
    //TOUCH_MOSI_DIR |= TOUCH_MOSI;											// TFT_MOSI pin as output
    //TOUCH_SCK_DIR |= TOUCH_SCK;											// TFT_SCK pin as output
    //TOUCH_MOSI_PORT |= TOUCH_MOSI;										// Hi state
    //TOUCH_SCK_PORT |= TOUCH_SCK;
#if USE_TOUCH_CS == 1														// If TFT_CS in use
    TOUCH_CS_DIR |= TOUCH_CS;
    TOUCH_CS_PORT |= TOUCH_CS;
#endif
    TOUCH_IRQ_DIR &= ~TOUCH_IRQ;											// Input
    TOUCH_IRQ_PORT |= TOUCH_IRQ;											// Pullup
};

static inline void XPT2046_activate(void)
{
    SPCR |= (1 << SPR1);													// F_CPU/32 (slow A/D conversion)
#if USE_TOUCH_CS == 1														// If TOUCH_CS in use
    TOUCH_CS_LO;
#endif
}

static inline void XPT2046_deactivate(void)
{
#if USE_TOUCH_CS == 1														// If TOUCH_CS in use
    TOUCH_CS_HI;
#endif
    SPCR &= ~(1 << SPR1);													// F_CPU/2
}

static inline void XPT2046_wr_cmd(uint8_t tx)
{
    SPI_rxtx(tx, TOUCH);
}

static inline uint8_t XPT2046_rd_data(uint8_t tx)
{
    uint8_t rx = SPI_rxtx(tx, TOUCH);
    return rx;
}

static inline int16_t XPT2046_rd16(uint8_t control)
{
    XPT2046_wr_cmd(control);
    int16_t value;
    value = XPT2046_rd_data(0);
    value <<= 8;
    value |= XPT2046_rd_data(0);
    return value;
}

static inline uint8_t XPT2046_rd8(uint8_t control)
{
    XPT2046_wr_cmd(control);
    return XPT2046_rd_data(0);
}

void XPT2046_rd_ee_cal(void)
{
    eeprom_read_block(&touch_cal, &ee_touch_cal, sizeof(cal_t));
}

void XPT2046_wr_ee_cal(void)
{
    eeprom_write_block(&touch_cal, &ee_touch_cal, sizeof(cal_t));
}

void XPT2046_rd_touch(void)
{
    int16_t temp;
    XPT2046_activate();
    touch.z1 = XPT2046_rd8(START_BIT | Z1_POS | MODE_8BIT | PD_MODE1);		// Start conversion for Z1 (8 bit, DFR mode)
    touch.z2 = XPT2046_rd8(START_BIT | Z2_POS | MODE_8BIT | PD_MODE1);		// Start conversion for Z2 (8 bit, DFR mode)
    touch.x_raw = 0;
    touch.y_raw = 0;

    for (uint8_t avg = 0; avg < TOUCH_AVG; avg++)
    {
        temp = XPT2046_rd16(START_BIT | X_POS | PD_MODE1);					// Start conversion for X (default 12 bit, DFR mode)
        temp /= X_MAX / TFT_HEIGHT;											// Scale raw->pixel
        touch.x_raw *= TOUCH_FILTER;										// Low-pass-avg filter
        touch.x_raw += temp;												// Low-pass-avg filter
        touch.x_raw /= (TOUCH_FILTER + 1);									// Low-pass-avg filter
        temp = XPT2046_rd16(START_BIT | Y_POS | PD_MODE1);					// Start conversion for Y (default 12 bit, DFR mode)
        temp /= Y_MAX / TFT_WIDTH;											// Scale raw->pixel
        touch.y_raw *= TOUCH_FILTER;										// Low-pass-avg filter
        touch.y_raw += temp;												// Low-pass-avg filter
        touch.y_raw /= (TOUCH_FILTER + 1);									// Low-pass-avg filter
    }

    // In horizontal position (SD slot faced down) exchange X <-> Y coordinates
    if ((rotation == LANDSCAPE) || (rotation == LANDSCAPE_REV)) swap(touch.x_raw, touch.y_raw);

    // When upside down, recalculate XY coordinates
    if (rotation == LANDSCAPE_REV)
    {
        touch.x_raw = TFT_WIDTH - touch.x_raw;
        touch.y_raw = TFT_HEIGHT - touch.y_raw;
    }

    // In vertical position, recalculate X or Y coordinate
    if (rotation == PORTRAIT) touch.x_raw = TFT_HEIGHT - touch.x_raw;

    if (rotation == PORTRAIT_REV) touch.y_raw = TFT_WIDTH - touch.y_raw;

    // This is to correctly identify touch
    if ((touch.y_raw * ((touch.z2 / (touch.z1 + 1))) < TOUCH_THRESHOLD)) touch.ok = true;
    else touch.ok = false;

    get_display_point();													// Apply calibration matrix
    XPT2046_deactivate();
}

/**********************************************************************
 *     Function: set_cal_matrix()
 *
 *  Description: Calling this function with valid input data
 *                in the display and screen input arguments
 *                causes the calibration factors between the
 *                screen and display points to be calculated,
 *                and the output - matrix_p - to be populated.
 *
 *               This function needs to be called ONLY when new
 *                calibration factors are desired.
 *
 *      Divider =  (Xs0 - Xs2)*(Ys1 - Ys2) - (Xs1 - Xs2)*(Ys0 - Ys2)
 *
 *                 (Xd0 - Xd2)*(Ys1 - Ys2) - (Xd1 - Xd2)*(Ys0 - Ys2)
 *            A = ---------------------------------------------------
 *                                   Divider
 *
 *
 *                 (Xs0 - Xs2)*(Xd1 - Xd2) - (Xd0 - Xd2)*(Xs1 - Xs2)
 *            B = ---------------------------------------------------
 *                                   Divider
 *
 *                 Ys0*(Xs2*Xd1 - Xs1*Xd2) +
 *                             Ys1*(Xs0*Xd2 - Xs2*Xd0) +
 *                                           Ys2*(Xs1*Xd0 - Xs0*Xd1)
 *            C = ---------------------------------------------------
 *                                   Divider
 *
 *                 (Yd0 - Yd2)*(Ys1 - Ys2) - (Yd1 - Yd2)*(Ys0 - Ys2)
 *            D = ---------------------------------------------------
 *                                   Divider
 *
 *                 (Xs0 - Xs2)*(Yd1 - Yd2) - (Yd0 - Yd2)*(Xs1 - Xs2)
 *            E = ---------------------------------------------------
 *                                   Divider
 *
 *                 Ys0*(Xs2*Yd1 - Xs1*Yd2) +
 *                             Ys1*(Xs0*Yd2 - Xs2*Yd0) +
 *                                           Ys2*(Xs1*Yd0 - Xs0*Yd1)
 *            F = ---------------------------------------------------
 *                                   Divider
 *
 */
void set_cal_matrix(point_t *sample_points)
{
    point_t temp_points[3];
    eeprom_read_block(&temp_points, &ee_cal_points, sizeof(ee_cal_points));		// Copy calibration to RAM
    touch_cal.divider = ((sample_points[0].x - sample_points[2].x) * (sample_points[1].y - sample_points[2].y)) -
                        ((sample_points[1].x - sample_points[2].x) * (sample_points[0].y - sample_points[2].y));
    touch_cal.An = ((temp_points[0].x - temp_points[2].x) * (sample_points[1].y - sample_points[2].y)) -
                   ((temp_points[1].x - temp_points[2].x) * (sample_points[0].y - sample_points[2].y));
    touch_cal.Bn = ((sample_points[0].x - sample_points[2].x) * (temp_points[1].x - temp_points[2].x)) -
                   ((temp_points[0].x - temp_points[2].x) * (sample_points[1].x - sample_points[2].x));
    touch_cal.Cn = (sample_points[2].x * temp_points[1].x - sample_points[1].x * temp_points[2].x) * sample_points[0].y +
                   (sample_points[0].x * temp_points[2].x - sample_points[2].x * temp_points[0].x) * sample_points[1].y +
                   (sample_points[1].x * temp_points[0].x - sample_points[0].x * temp_points[1].x) * sample_points[2].y;
    touch_cal.Dn = ((temp_points[0].y - temp_points[2].y) * (sample_points[1].y - sample_points[2].y)) -
                   ((temp_points[1].y - temp_points[2].y) * (sample_points[0].y - sample_points[2].y));
    touch_cal.En = ((sample_points[0].x - sample_points[2].x) * (temp_points[1].y - temp_points[2].y)) -
                   ((temp_points[0].y - temp_points[2].y) * (sample_points[1].x - sample_points[2].x));
    touch_cal.Fn = (sample_points[2].x * temp_points[1].y - sample_points[1].x * temp_points[2].y) * sample_points[0].y +
                   (sample_points[0].x * temp_points[2].y - sample_points[2].x * temp_points[0].y) * sample_points[1].y +
                   (sample_points[1].x * temp_points[0].y - sample_points[0].x * temp_points[1].y) * sample_points[2].y;
}

/**********************************************************************
 *     Function: getDisplayPoint()
 *
 *  Description: Given a valid set of calibration factors and a point
 *                value reported by the touch screen, this function
 *                calculates and returns the true (or closest to true)
 *                display point below the spot where the touch screen
 *                was touched.
 *
 *  Argument(s): touch_p (input) - Pointer to the reported touch
 *                                 screen point
 */
void get_display_point(void)
{
    /* Operation order is important since we are doing integer */
    /*  math. Make sure you add all terms together before      */
    /*  dividing, so that the remainder is not rounded off     */
    /*  prematurely.                                           */
    touch.x_cal = ((touch_cal.An * touch.x_raw) + (touch_cal.Bn * touch.y_raw) +
                   touch_cal.Cn) / touch_cal.divider;
    touch.y_cal = ((touch_cal.Dn * touch.x_raw) + (touch_cal.En * touch.y_raw) +
                   touch_cal.Fn) / touch_cal.divider;
}
