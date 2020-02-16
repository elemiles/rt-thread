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

static struct rt_i2c_bus_device *i2c_bus = RT_NULL;
struct rt_semaphore sem_energy;
static void eeprom_thread_entry(void *parameter);
static void eeprom_example(void);
typedef struct energy_packet* energy_t;


struct energy_packet
{
    uint32_t voltage;
    uint32_t current;
    uint32_t active_power;
    uint32_t active_energy;
    uint16_t checksum;
};

static struct energy_packet meter_data =
{
    .voltage = 0,
    .current = 0,
    .active_power = 0,
    .active_energy = 0,
    .checksum = 0
};


/* defined the LED0 pin: PC0 */
#define LED0_PIN    GET_PIN(A, 8)
#define RT_EEPROM_THREAD_STACK_SIZE 1024
#define RT_EEPROM_THREAD_PRIORITY 9
#define RT_ENERGY_THREAD_STACK_SIZE 1024
#define RT_ENERGY_THREAD_PRIORITY 10

void energy_process(void *ptr)
{
    struct energy_packet* buf= (struct energy_packet*)ptr;
    buf->voltage = 220;
    buf->current = 100;
    buf->active_power = buf->voltage*buf->current;
    buf->active_energy += buf->active_energy + (uint32_t)(buf->active_power/100.0);
}

void eeprom_thread_entry(void *parameter)
{
    struct rt_i2c_msg msgs;
    
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find("i2c1");
    
    while (1)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        
        rt_sem_take(&sem_energy, RT_WAITING_FOREVER);
        energy_process(&meter_data.voltage);
        rt_sem_release(&sem_energy);
        
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(2000);
        
        msgs.addr = 0;
        msgs.flags = RT_I2C_WR;
        msgs.buf = (uint8_t *)&meter_data.voltage;
        msgs.len = sizeof(meter_data);
        rt_i2c_transfer(i2c_bus, &msgs, 1);
    }
}

void energyshow_thread_entry(void *parameter)
{
    struct rt_i2c_msg msgs;
    uint32_t buf[4];
    struct energy_packet * ptr = (struct energy_packet *)&buf[0];

    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find("i2c1");
    
    while (1)
    {
        rt_thread_mdelay(1000);
        
        msgs.addr = 0;
        msgs.flags = RT_I2C_RD;
        msgs.buf = (uint8_t *)&buf[0];
        msgs.len = sizeof(meter_data);
        rt_i2c_transfer(i2c_bus, &msgs, 1);
        
        rt_kprintf("voltage: %d\n\rcurrent:%d\n\ractive power:%d\n\ractive energy:%d", \
        ptr->voltage, ptr->current, ptr->active_power, ptr->active_energy);
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

    rt_thread_startup(tid);
    rt_kprintf("eeprom thread startup!\n\r");
    tid = rt_thread_create("energyshow", energyshow_thread_entry, RT_NULL,
    RT_ENERGY_THREAD_STACK_SIZE, RT_ENERGY_THREAD_PRIORITY, 0);
    RT_ASSERT(tid != RT_NULL);
    rt_thread_startup(tid);
    rt_kprintf("energyshow thread startup!\n\r");
}




int main(void)
{
    //eeprom_example();
    return RT_EOK;
}


MSH_CMD_EXPORT(eeprom_example, eeprom example);
