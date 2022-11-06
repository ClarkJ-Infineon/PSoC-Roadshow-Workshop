/******************************************************************************
* File Name:   main.c
*******************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/* FreeRTOS headers */
#include <FreeRTOS.h>
#include <task.h>

/* RTOS related macros. */
#define MAIN_TASK_STACK_SIZE        (10 * 1024)
#define MAIN_TASK_PRIORITY          (6)


/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void main_task(void *arg);

/*******************************************************************************
* Global Variables
*******************************************************************************/
TaskHandle_t main_task_handle;
volatile int uxTopUsedPriority;
uint32_t cm4_led_value = CYBSP_LED_STATE_OFF;


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for CM4 CPU and serves as a template for a basic
* FreeRTOS application.  This function initializes the device, create a FreeRTOS
* task, and starts the FreeRTOS scheduler.
*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
	cy_rslt_t result;

	/* This enables RTOS aware debugging in OpenOCD */
	uxTopUsedPriority = configMAX_PRIORITIES - 1;

	/* Initialize the device and board peripherals */
	result = cybsp_init() ;
	if (result != CY_RSLT_SUCCESS)
	{
		CY_ASSERT(0);
	}

	/* Enable global interrupts */
	__enable_irq();

	// TODO Initialize Serial Flash


	cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
	printf("\x1b[2J\x1b[;H");
	printf("Hello PSoC 6 Technical Workshop\n");

	xTaskCreate(main_task, "Main Task", MAIN_TASK_STACK_SIZE , NULL, MAIN_TASK_PRIORITY, &main_task_handle);

	/* Start the FreeRTOS scheduler */
	vTaskStartScheduler();

	/* Should never get here */
	CY_ASSERT(0);
}


/*******************************************************************************
* Function Name: main_task
********************************************************************************
* Summary:
* This is the main FreeRTOS task.  This task will be used to within the
* workshop to configure and start the WiFi Access Point and HTTP Server.
* Within the for loop, the LED status will be monitor and updated as needed
* based on FreeRTOS task notifications.
*
* Parameters:
*  arg: task argument
*
* Return:
*  None
*
*******************************************************************************/
void main_task(void *arg)
{
	(void) arg;

	printf("This is the main task\n");

	// TODO Get IPC Message Queue Handle


	// TODO Create Access Point


	// TODO Create HTTP Server


	for (;;)
	{
		// Toggle LED variable
    	if (cm4_led_value == CYBSP_LED_STATE_ON) {
    		cm4_led_value = CYBSP_LED_STATE_OFF;
    	} else {
    		cm4_led_value = CYBSP_LED_STATE_ON;
    	}

		vTaskDelay(100);
	}
}


/* [] END OF FILE */
