/*
Copyright 2019 Massdrop Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "samd51j18a.h"
#include "tmk_core/common/keyboard.h"

#include "report.h"
#include "host.h"
#include "host_driver.h"
#include "keycode_config.h"
#include <string.h>
#include "quantum.h"


//From protocol directory
#include "arm_atsam_protocol.h"

//From keyboard's directory
#include "config_led.h"

volatile char embed_version[] = COMPILE_VERSION;

uint8_t g_usb_state = USB_FSMSTATUS_FSMSTATE_OFF_Val;   //Saved USB state from hardware value to detect changes

void main_subtasks(void);
uint8_t keyboard_leds(void);
void send_keyboard(report_keyboard_t *report);
void send_mouse(report_mouse_t *report);
void send_system(uint16_t data);
void send_consumer(uint16_t data);

host_driver_t arm_atsam_driver = {
    keyboard_leds,
    send_keyboard,
    send_mouse,
    send_system,
    send_consumer
};

uint8_t led_states;

uint8_t keyboard_leds(void)
{
#ifdef NKRO_ENABLE
    if (keymap_config.nkro)
        return udi_hid_nkro_report_set;
    else
#endif //NKRO_ENABLE
        return udi_hid_kbd_report_set;
}

void send_keyboard(report_keyboard_t *report)
{
    uint32_t irqflags;

#ifdef NKRO_ENABLE
    if (!keymap_config.nkro)
    {
#endif //NKRO_ENABLE
        while (udi_hid_kbd_b_report_trans_ongoing) { main_subtasks(); } //Run other tasks while waiting for USB to be free

        irqflags = __get_PRIMASK();
        __disable_irq();
        __DMB();

        memcpy(udi_hid_kbd_report, report->raw, UDI_HID_KBD_REPORT_SIZE);
        udi_hid_kbd_b_report_valid = 1;
        udi_hid_kbd_send_report();

        __DMB();
        __set_PRIMASK(irqflags);
#ifdef NKRO_ENABLE
    }
    else
    {
        while (udi_hid_nkro_b_report_trans_ongoing) { main_subtasks(); } //Run other tasks while waiting for USB to be free

        irqflags = __get_PRIMASK();
        __disable_irq();
        __DMB();

        memcpy(udi_hid_nkro_report, report->raw, UDI_HID_NKRO_REPORT_SIZE);
        udi_hid_nkro_b_report_valid = 1;
        udi_hid_nkro_send_report();

        __DMB();
        __set_PRIMASK(irqflags);
    }
#endif //NKRO_ENABLE
}

void send_mouse(report_mouse_t *report)
{
#ifdef MOUSEKEY_ENABLE
    uint32_t irqflags;

    irqflags = __get_PRIMASK();
    __disable_irq();
    __DMB();

    memcpy(udi_hid_mou_report, report, UDI_HID_MOU_REPORT_SIZE);
    udi_hid_mou_b_report_valid = 1;
    udi_hid_mou_send_report();

    __DMB();
    __set_PRIMASK(irqflags);
#endif //MOUSEKEY_ENABLE
}

void send_system(uint16_t data)
{
#ifdef EXTRAKEY_ENABLE
    uint32_t irqflags;

    irqflags = __get_PRIMASK();
    __disable_irq();
    __DMB();

    udi_hid_exk_report.desc.report_id = REPORT_ID_SYSTEM;
    if (data != 0) data = data - SYSTEM_POWER_DOWN + 1;
    udi_hid_exk_report.desc.report_data = data;
    udi_hid_exk_b_report_valid = 1;
    udi_hid_exk_send_report();

    __DMB();
    __set_PRIMASK(irqflags);
#endif //EXTRAKEY_ENABLE
}

void send_consumer(uint16_t data)
{
#ifdef EXTRAKEY_ENABLE
    uint32_t irqflags;

    irqflags = __get_PRIMASK();
    __disable_irq();
    __DMB();

    udi_hid_exk_report.desc.report_id = REPORT_ID_CONSUMER;
    udi_hid_exk_report.desc.report_data = data;
    udi_hid_exk_b_report_valid = 1;
    udi_hid_exk_send_report();

    __DMB();
    __set_PRIMASK(irqflags);
#endif //EXTRAKEY_ENABLE
}

void main_subtask_usb_state(void)
{
    static uint64_t fsmstate_on_delay = 0;                          //Delay timer to be sure USB is actually operating before bringing up hardware
    uint8_t fsmstate_now = USB->DEVICE.FSMSTATUS.reg;               //Current state from hardware register

    if (fsmstate_now == USB_FSMSTATUS_FSMSTATE_SUSPEND_Val)         //If USB SUSPENDED
    {
        fsmstate_on_delay = 0;                                      //Clear ON delay timer

        if (g_usb_state != USB_FSMSTATUS_FSMSTATE_SUSPEND_Val)      //If previously not SUSPENDED
        {
            suspend_power_down();                                   //Run suspend routine
            g_usb_state = fsmstate_now;                             //Save current USB state
        }
    }
    else if (fsmstate_now == USB_FSMSTATUS_FSMSTATE_SLEEP_Val)      //Else if USB SLEEPING
    {
        fsmstate_on_delay = 0;                                      //Clear ON delay timer

        if (g_usb_state != USB_FSMSTATUS_FSMSTATE_SLEEP_Val)        //If previously not SLEEPING
        {
            suspend_power_down();                                   //Run suspend routine
            g_usb_state = fsmstate_now;                             //Save current USB state
        }
    }
    else if (fsmstate_now == USB_FSMSTATUS_FSMSTATE_ON_Val)         //Else if USB ON
    {
        if (g_usb_state != USB_FSMSTATUS_FSMSTATE_ON_Val)           //If previously not ON
        {
            if (fsmstate_on_delay == 0)                             //If ON delay timer is cleared
            {
                fsmstate_on_delay = timer_read64() + 250;           //Set ON delay timer
            }
            else if (timer_read64() > fsmstate_on_delay)            //Else if ON delay timer is active and timed out
            {
                suspend_wakeup_init();                              //Run wakeup routine
                g_usb_state = fsmstate_now;                         //Save current USB state
            }
        }
    }
    else                                                            //Else if USB is in a state not being tracked
    {
        fsmstate_on_delay = 0;                                      //Clear ON delay timer
    }
}

void main_subtask_power_check(void)
{
    static uint64_t next_5v_checkup = 0;

    if (timer_read64() >= next_5v_checkup)
    {
        next_5v_checkup = timer_read64() + POWER_CHECK_INTERVAL;

        g_v_5v = adc_get(ADC_5V);
        g_v_5v_avg = (((float)V_5V_AVGS - 1) / (float)V_5V_AVGS) * g_v_5v_avg + (1 / (float)V_5V_AVGS) * (float)g_v_5v;

        power_run(); //Check up on 5v bus voltage for spikes or low conditions

#ifdef RGB_MATRIX_ENABLE
        gcr_compute();
#endif
    }
}

void main_subtask_usb_extra_device(void)
{
    static uint64_t next_usb_checkup = 0;

    if (timer_read64() >= next_usb_checkup)
    {
        next_usb_checkup = timer_read64() + USBC_CFG_PERIOD;

        USB_HandleExtraDevice();
    }
}

void main_subtasks(void)
{
    main_subtask_usb_state();
    main_subtask_power_check();
    main_subtask_usb_extra_device();
}

int main(void)
{
    DBG_LED_ENA;
    DBG_1_ENA;
    DBG_1_OFF;
    DBG_2_ENA;
    DBG_2_OFF;
    DBG_3_ENA;
    DBG_3_OFF;

    debug_code_init();

    CLK_init();

    ADC_init();

#ifndef NO_MD_SR_EXT
    SR_EXP_Init();
#endif
#ifdef RGB_MATRIX_ENABLE
    i2c1_init();
#endif // RGB_MATRIX_ENABLE
    power_init();

    matrix_init();

    USB2422_init();

    DBGC(DC_MAIN_UDC_START_BEGIN);
    udc_start();
    DBGC(DC_MAIN_UDC_START_COMPLETE);

    DBGC(DC_MAIN_CDC_INIT_BEGIN);
    CDC_init();
    DBGC(DC_MAIN_CDC_INIT_COMPLETE);

#ifndef NO_MD_USB2422		// Do we need alternate for non HUB products?  ***TBD
    while (USB2422_Port_Detect_Init() == 0) {}
#endif
    DBG_LED_OFF;

#ifdef RGB_MATRIX_ENABLE
    while (I2C3733_Init_Control() != 1) {}
    while (I2C3733_Init_Drivers() != 1) {}

    I2C_DMAC_LED_Init();

    i2c_led_q_init();

    for (uint8_t drvid = 0; drvid < ISSI3733_DRIVER_COUNT; drvid++)
        I2C_LED_Q_ONOFF(drvid); //Queue data
#endif // RGB_MATRIX_ENABLE

    keyboard_setup();

    keyboard_init();

    host_set_driver(&arm_atsam_driver);

#if defined(CONSOLE_ENABLE)
    uint64_t next_print = 0;
#endif //CONSOLE_ENABLE

    debug_code_disable();

    usbc_enable();

    while (1)
    {
        main_subtasks(); //Note these tasks will also be run while waiting for USB keyboard polling intervals

        if (g_usb_state == USB_FSMSTATUS_FSMSTATE_SUSPEND_Val || g_usb_state == USB_FSMSTATUS_FSMSTATE_SLEEP_Val)
        {
            if (suspend_wakeup_condition())
            {
                udc_remotewakeup(); //Send remote wakeup signal
                wait_ms(50);
            }

            continue;
        }

        keyboard_task();

#ifdef RAW_ENABLE
        raw_hid_task();
#endif

#if defined(CONSOLE_ENABLE)
        if (timer_read64() > next_print)
        {
            next_print = timer_read64() + 1;

            //Add any debug information here that you want to see very often

            //Corrected CC values for host port detection debugging
            //dprintf("%4u %4u %4u %4u\n",ADC_CC_5VCOR(g_v_5v, adc_get(ADC_C1A5)),
            //                            ADC_CC_5VCOR(g_v_5v, adc_get(ADC_C1B5)),
            //                            ADC_CC_5VCOR(g_v_5v, adc_get(ADC_C2A5)),
            //                            ADC_CC_5VCOR(g_v_5v, adc_get(ADC_C2B5)));

            //USB state and CC line monitoring
            //dprintf("%4u %4u %4u %4u %2u %1u\n",usbc_cc_a5_v,usbc_cc_b5_v,(uint16_t)usbc_cc_a5_v_avg,(uint16_t)usbc_cc_b5_v_avg,usbc.state,usbc.state == USB_STATE_ATTACHED_SRC ? 1 : 0);

            //Power manager monitoring
            dprintf("%4u %4u %3u %3u %3u %3u %3u\n", g_v_5v, (uint16_t)g_v_5v_avg, gcr_desired,(uint8_t)gcr_actual, gcr_actual_last, usbc.state, I2C3733_Control_Get());
            //dprintf("%u %u %u %f %u %u %u\n",
            //		led_animation_breathing,
            //		led_animation_id,
			//		led_lighting_mode,
			//		led_animation_speed,
			//		led_animation_breathe_cur,
			//		led_animation_direction,
			//		breathe_dir );

            //dprintf("\n");
            //for (uint8_t i = 0; i < ISSI3733_LED_COUNT; i++)
            //{
            //	dprintf("%2u: %u, %u, %u\n", i, led_buffer[i].r, led_buffer[i].g, led_buffer[i].b);
            //}

        }
#endif //CONSOLE_ENABLE

    }

    return 1;
}


