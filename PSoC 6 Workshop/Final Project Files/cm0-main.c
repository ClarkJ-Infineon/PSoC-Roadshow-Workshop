/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for CM0+ in the the Dual CPU Empty 
*              Application for ModusToolbox.
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2020-2022, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

#include "cy_pdl.h"
#include "cycfg.h"
#include "cybsp.h"
#include "cyhal.h"

int main(void)
{
    /* Enable global interrupts */
    __enable_irq();
    
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    // Variable declaration for message queue and payload
    cyhal_ipc_t cm0_msg_queue;
    uint32_t cm0_led_value = CYBSP_LED_STATE_ON;

    // Pointer creation and shared memory allocatino for queue pool and handle
    void* queue_pool;
    cyhal_ipc_queue_t* queue_handle;
    CYHAL_IPC_QUEUE_POOL_ALLOC(queue_pool,1UL,sizeof(cm0_led_value));
    CYHAL_IPC_QUEUE_HANDLE_ALLOC(queue_handle);

    // Configuration of Message Queue objecy
    queue_handle->channel_num = CYHAL_IPC_CHAN_0;
    queue_handle->queue_num = 1UL;
    queue_handle->queue_pool = queue_pool;
    queue_handle->num_items = 1UL;
    queue_handle->item_size = sizeof(cm0_led_value);

    //Initialization of Message Queue
    cyhal_ipc_queue_init(&cm0_msg_queue, queue_handle);

    // Initialize GPIO using HAL APIs
    cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, cm0_led_value);

    /* Enable CM4. CY_CORTEX_M4_APPL_ADDR must be updated if CM4 memory layout is changed. */
    Cy_SysEnableCM4(CY_CORTEX_M4_APPL_ADDR);

    for (;;)
    {
    	// Wait for queue message to be recieved and get message from queue
    	cyhal_ipc_queue_get(&cm0_msg_queue, &cm0_led_value, CYHAL_IPC_NEVER_TIMEOUT);

    	// Drive value out to GPIO
    	cyhal_gpio_write(CYBSP_USER_LED, cm0_led_value);

    }
}

/* [] END OF FILE */
