/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-27     balanceTWK   first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

static void eeprom_thread_entry(void *parameter);
static void eeprom_example(void);

/* defined the LED0 pin: PC0 */
#define LED0_PIN    GET_PIN(A, 8)
#define RT_EEPROM_THREAD_STACK_SIZE 1024
#define RT_EEPROM_THREAD_PRIORITY 9

void eeprom_thread_entry(void *parameter)
{
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    while (1)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}

void eeprom_example(void)
{
    rt_thread_t tid;

    #ifdef RT_USING_HEAP
    tid = rt_thread_create("eeprom_thread", eeprom_thread_entry, RT_NULL,
        RT_EEPROM_THREAD_STACK_SIZE, RT_EEPROM_THREAD_PRIORITY, 0);
    //tid = RT_NULL;
    RT_ASSERT(tid != RT_NULL);
    #else
    rt_err_t result;

    tid = &main_thread;
    result = rt_thread_init(tid, "main", main_thread_entry, RT_NULL,
    main_stack, sizeof(main_stack), RT_MAIN_THREAD_PRIORITY, 20);
    RT_ASSERT(result == RT_EOK);

    /* if not define RT_USING_HEAP, using to eliminate the warning */
    (void)result;
    #endif
    
    rt_kprintf("eeprom thread startup!\n\r");

    rt_thread_startup(tid);
}




int main(void)
{
    //eeprom_example();
    return RT_EOK;
}


MSH_CMD_EXPORT(eeprom_example, eeprom example);
