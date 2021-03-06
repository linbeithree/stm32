#include <stdlib.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "rtc.h"

#include "eventlogger.h"
#include "MPU9150.h"
#include "psas_rtc.h"
#include "psas_sdclog.h"
#include "usbdetail.h"

#include "mpu9150.h"


#define UNUSED __attribute__((unused))

/*
 * Configuration
 * ============= **************************************************************
 */

const I2CConfig mpu9150_config = {
    OPMODE_I2C,
    // i2c clock speed. Test at 400000 when r=4.7k
    400000,
    FAST_DUTY_CYCLE_2
};

const mpu9150_connect mpu9150_connections = {
    GPIOF, // i2c sda port
    0,     // i2c_sda_pad
    GPIOF, // i2c_scl_port
    1,     // i2c scl pad
    GPIOF, // interrupt port
    2,     // interrupt pad;
};

const EXTConfig extcfg = {
    {
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_FALLING_EDGE | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOF, mpu9150_ISR}, // F13
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL},
        {EXT_CH_MODE_DISABLED, NULL}
    }
};



/*
 * Threads
 * ======= *********************************************************************
 */

/*
 * LED Blinker Thread
 *
 * Blink slowly when the SD card is not inserted, and quickly when it is.
 */

static WORKING_AREA(wa_thread_blinker, 64);

static msg_t blinker(void *_ UNUSED) {
    chRegSetThreadName("blinker");

    while (TRUE) {
        palTogglePad(GPIOC, GPIOC_LED);
        chThdSleepMilliseconds(fs_ready ? 125 : 500);
    }

    return -1;
}


/*
 * MPU9150 Dispatch Thread Storage
 * See mpu9150.c for implementation.
 */

static WORKING_AREA(wa_thread_mpu9150_dispatch, 512);



/*
 * Main
 * ==== ************************************************************************
 */

int main(void) {
    static const evhandler_t sdc_handlers[] = { sdc_insert_handler, sdc_remove_handler };
    struct EventListener insertion_listener, removal_listener;

    /*!
     * System initializations.
     * - HAL initialization, this also initializes the configured device drivers
     *   and performs the board-specific initializations.
     * - Kernel initialization, the main() function becomes a thread and the
     *   RTOS is active.
     */
    halInit();
    chSysInit();

    psas_rtc_lld_init();
    psas_rtc_set_fc_boot_mark(&RTCD1);

    palSetPad(GPIOC, GPIOC_LED);

    /*!
     * I2C2 I/O pins setup
     */
    palSetPadMode( mpu9150_connections.i2c_sda_port
                 , mpu9150_connections.i2c_sda_pad
                 , PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN
                                         | PAL_STM32_OSPEED_HIGHEST
                                         | PAL_STM32_PUDR_FLOATING
                 );
    palSetPadMode( mpu9150_connections.i2c_scl_port
                 , mpu9150_connections.i2c_scl_pad
                 , PAL_MODE_ALTERNATE(4) | PAL_STM32_OSPEED_HIGHEST
                                         | PAL_STM32_PUDR_FLOATING
            );
    palSetPad(mpu9150_connections.i2c_scl_port,  mpu9150_connections.i2c_scl_pad);

    /*!
     * Activates the SDC driver 1 using default
     * configuration.
     */
    sdcStart(&SDCD1, NULL);

    /*!
     * Activates SD card insertion monitor & registers SD card events.
     */
    sdc_tmr_init(&SDCD1);
    chEvtRegister(&sdc_inserted_event, &insertion_listener, 0);
    chEvtRegister(&sdc_removed_event,  &removal_listener,   1);

    /*!
     * Starts the LED blinker thread
     */
    chThdCreateStatic( wa_thread_blinker
                     , sizeof(wa_thread_blinker)
                     , NORMALPRIO
                     , blinker
                     , NULL
                     );

    /*!
     * Starts the eventlogger
     */
    eventlogger_init();

    /*!
     * Initializes MPU9150 & associated I2C connection
     */
    mpu9150_start(&I2CD2);
    i2cStart(mpu9150_driver.i2c_instance, &mpu9150_config);

    /*!
     * Starts the mpu9150 dispatch thread
     */
    chThdCreateStatic( wa_thread_mpu9150_dispatch
                     , sizeof (wa_thread_mpu9150_dispatch)
                     , NORMALPRIO
                     , Thread_mpu9150_int_dispatch
                     , NULL
                     );

    /*! Activates the EXT driver 1. */
    extStart(&EXTD1, &extcfg);

    // It is possible the card is already inserted. Check now by calling insert handler directly.
    sdc_insert_handler(0);

    /*
     * Wait for events to fire
     */
    while (true) {
        chEvtDispatch(sdc_handlers, chEvtWaitOne(ALL_EVENTS));
    }

    exit(0);
}
