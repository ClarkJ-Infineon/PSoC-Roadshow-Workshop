/******************************************************************************
* File Name: main.c
*
* Description: This is the source code for the PSoC 4 MCU Hello World Example
*              for ModusToolbox.
*
* Related Document: See README.md
*
*******************************************************************************
* Copyright 2020-2021, Cypress Semiconductor Corporation (an Infineon company) or
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

/*******************************************************************************
 * Include header files
 ******************************************************************************/
#include "cy_pdl.h"
#include "cybsp.h"

#include "cycfg_capsense.h"
#include "cy_crypto.h"

#include "game.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define LED_DELAY_MS              (500u)
#define CY_ASSERT_FAILED          (0u)

uint32_t rndNum = 0;

void process_touch(void);

void Button_Callback(void)
{
	Cy_GPIO_ClearInterrupt(SW1_PORT, SW1_PIN);
}

volatile bool timer_expired = false;
void Timer_Callback(void)
{
	Cy_TCPWM_ClearInterrupt(Timer_HW, Timer_NUM, CY_TCPWM_INT_ON_TC);
	timer_expired = true;
}

void snippet_Cy_CapSense_IntHandler(void)
{
    Cy_CapSense_InterruptHandler(CYBSP_CSD_HW, &cy_capsense_context);
}

cy_en_syspm_status_t Sleep_Callback(cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode);

/* SysPm callback params */
cy_stc_syspm_callback_params_t callbackParams = {NULL,NULL};

/* Callback declaration for Sleep mode */
cy_stc_syspm_callback_t sleep_cb    =
{
		&Sleep_Callback,              /* Callback function */
		CY_SYSPM_SLEEP,               /* Callback type */
		0,                            /* Skip mode */
		&callbackParams,              /* Callback params */
		NULL, NULL,					  /* For internal usage */
};


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  System entrance point. This function performs
*  - initial setup of device
*  - configure the SCB block as UART interface
*  - prints out "Hello World" via UART interface
*  - Blinks an LED under firmware control at 1 Hz
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
    cy_stc_scb_uart_context_t CYBSP_UART_context;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(CY_ASSERT_FAILED);
    }

    /* Configure and enable the UART peripheral */
    Cy_SCB_UART_Init(CYBSP_UART_HW, &CYBSP_UART_config, &CYBSP_UART_context);
    Cy_SCB_UART_Enable(CYBSP_UART_HW);


    /* Enable global interrupts */
    __enable_irq();

    /* Code to register a GPIO interrupt */
    cy_stc_sysint_t ButtonIRQ_cfg = { .intrPriority = 3, .intrSrc = SW1_IRQ };
    Cy_SysInt_Init(&ButtonIRQ_cfg, Button_Callback);
    NVIC_EnableIRQ(SW1_IRQ);

    /* Initialize and enable TCPWM Counter */
    Cy_TCPWM_Counter_Init(Timer_HW, Timer_NUM, &Timer_config);
    Cy_TCPWM_Counter_Enable(Timer_HW, Timer_NUM);

    /* Start TCPWM Timer */
    Cy_TCPWM_TriggerStart(Timer_HW, Timer_MASK);


    /* Register callback for Timer interrupt */
    cy_stc_sysint_t TimerIRQ_cfg = { .intrPriority = 3, .intrSrc = Timer_IRQ };
    Cy_SysInt_Init(&TimerIRQ_cfg, Timer_Callback);
    NVIC_EnableIRQ(Timer_IRQ);


    const cy_stc_sysint_t CapSense_ISR_cfg =
    {
        .intrSrc = csd_interrupt_IRQn,  /* CM0+ interrupt is NVIC #2 */
        .intrPriority = 3u,             /* Interrupt priority is 3 */
    };


    /* Capture the CAPSENSE&trade; HW block and initialize it to the default state. */
    Cy_CapSense_Init(&cy_capsense_context);
    /* Initialize CAPSENSE&trade; interrupt */
    Cy_SysInt_Init(&CapSense_ISR_cfg, &snippet_Cy_CapSense_IntHandler);
    NVIC_ClearPendingIRQ(CapSense_ISR_cfg.intrSrc);
    NVIC_EnableIRQ(CapSense_ISR_cfg.intrSrc);
    /* Initialize the CAPSENSE&trade; firmware modules. */
    Cy_CapSense_Enable(&cy_capsense_context);


    Cy_Crypto_Enable(CRYPTO);

    /* Register Callback */
    Cy_SysPm_RegisterCallback(&sleep_cb);

    //Cy_SmartIO_Init(BREATHING_HW, &BREATHING_config);
    //Cy_SmartIO_Enable(BREATHING_HW);

    //Cy_TCPWM_PWM_Init(PWM2_HW, PWM2_NUM, &PWM2_config);
    //Cy_TCPWM_PWM_Enable(PWM2_HW, PWM2_NUM);

    //Cy_TCPWM_PWM_Init(PWM3_HW, PWM3_NUM, &PWM3_config);
    //Cy_TCPWM_PWM_Enable(PWM3_HW, PWM3_NUM);

    /* Send a string over serial terminal */
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\x1b[2J\x1b[;H");
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "Hello world\r\n");

    game_init();

    for(;;)
    {

    	/* Scan and Process CAPSENSE widgets */
    	if (CY_CAPSENSE_NOT_BUSY == Cy_CapSense_IsBusy(&cy_capsense_context)) {
    		if (CY_CAPSENSE_STATUS_SUCCESS == Cy_CapSense_ProcessAllWidgets(&cy_capsense_context) ) {
    			process_touch();
    		}
    		Cy_CapSense_ScanAllWidgets(&cy_capsense_context);
    	}



    	if (timer_expired) {
    		timer_expired = false;

    		/* IF statement to check is any CYBSP_LED_BTNx is still on */
    		if ( (Cy_GPIO_Read(CYBSP_LED_BTN0_PORT, CYBSP_LED_BTN0_NUM) == CYBSP_LED_STATE_ON) || \
    		     (Cy_GPIO_Read(CYBSP_LED_BTN1_PORT, CYBSP_LED_BTN1_NUM) == CYBSP_LED_STATE_ON) || \
    			 (Cy_GPIO_Read(CYBSP_LED_BTN2_PORT, CYBSP_LED_BTN2_NUM) == CYBSP_LED_STATE_ON) )
    		{
    			// Game Over - Too Slow
    			Cy_SCB_UART_PutString(CYBSP_UART_HW, "Too Slow\r\n");
    			game_over();
    			Cy_SysPm_CpuEnterSleep();
    		} else {
    			// Good Job - Start next round
    			game_advance();

    			Cy_Crypto_Trng(CRYPTO, 32UL, &rndNum);

    			/* Code to turn on CYBSP_LED_BTNx corresponding to randomly generated number */
    			if (rndNum%3 == 0) {
    				Cy_GPIO_Write(CYBSP_LED_BTN0_PORT, CYBSP_LED_BTN0_NUM, CYBSP_LED_STATE_ON);
    			} else if (rndNum%3 == 1) {
    				Cy_GPIO_Write(CYBSP_LED_BTN1_PORT, CYBSP_LED_BTN1_NUM, CYBSP_LED_STATE_ON);
    			} else {
    				Cy_GPIO_Write(CYBSP_LED_BTN2_PORT, CYBSP_LED_BTN2_NUM, CYBSP_LED_STATE_ON);
    			}
    		}
    	}
    }
}

/* Process touch events and respond accordingly */
void process_touch(void)
{
	uint32_t button0 = 0;
	static uint32_t prev_button0 = 0;

	button0 = Cy_CapSense_IsWidgetActive(CY_CAPSENSE_BUTTON0_WDGT_ID, &cy_capsense_context);

	if ( ( button0 != 0 ) && ( button0 != prev_button0 ) )
	{
		if (Cy_GPIO_Read(CYBSP_LED_BTN0_PORT, CYBSP_LED_BTN0_NUM) == CYBSP_LED_STATE_ON) {
			Cy_GPIO_Write(CYBSP_LED_BTN0_PORT, CYBSP_LED_BTN0_NUM, CYBSP_LED_STATE_OFF);
		} else  {
			Cy_SCB_UART_PutString(CYBSP_UART_HW, "Wrong Button\r\n");
			game_over();
			Cy_SysPm_CpuEnterSleep();
		}
	}
	prev_button0 = button0;

	uint32_t button1 = 0;
	static uint32_t prev_button1 = 0;

	button1 = Cy_CapSense_IsWidgetActive(CY_CAPSENSE_BUTTON1_WDGT_ID, &cy_capsense_context);

	if ( ( button1 != 0 ) && ( button1 != prev_button1 ) )
	{
		if (Cy_GPIO_Read(CYBSP_LED_BTN1_PORT, CYBSP_LED_BTN1_NUM) == CYBSP_LED_STATE_ON) {
			Cy_GPIO_Write(CYBSP_LED_BTN1_PORT, CYBSP_LED_BTN1_NUM, CYBSP_LED_STATE_OFF);
		} else  {
			Cy_SCB_UART_PutString(CYBSP_UART_HW, "Wrong Button\r\n");
			game_over();
			Cy_SysPm_CpuEnterSleep();
		}
	}
	prev_button1 = button1;

	uint32_t button2 = 0;
	static uint32_t prev_button2 = 0;

	button2 = Cy_CapSense_IsWidgetActive(CY_CAPSENSE_BUTTON2_WDGT_ID, &cy_capsense_context);

	if ( ( button2 != 0 ) && ( button2 != prev_button2 ) )
	{
		if (Cy_GPIO_Read(CYBSP_LED_BTN2_PORT, CYBSP_LED_BTN2_NUM) == CYBSP_LED_STATE_ON) {
			Cy_GPIO_Write(CYBSP_LED_BTN2_PORT, CYBSP_LED_BTN2_NUM, CYBSP_LED_STATE_OFF);
		} else  {
			Cy_SCB_UART_PutString(CYBSP_UART_HW, "Wrong Button\r\n");
			game_over();
			Cy_SysPm_CpuEnterSleep();
		}
	}
	prev_button2 = button2;

}

/* Low-Power Callback Sleep_Callback template */
cy_en_syspm_status_t Sleep_Callback(cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode)
{
	cy_en_syspm_status_t retVal = CY_SYSPM_FAIL;
	if (callbackParams != NULL) {}

	switch(mode)
	{
	case CY_SYSPM_CHECK_READY:
		/* Add code here to check for readiness of entering sleep modem */

		Cy_SysLib_Delay(10);

		retVal = CY_SYSPM_SUCCESS;
		break;

	case CY_SYSPM_BEFORE_TRANSITION:
		/* Add code here to execute prior to entering sleep mode */

		Cy_GPIO_Write(CYBSP_USER_LED_PORT, CYBSP_USER_LED_NUM, 0);
		Cy_GPIO_Write(CYBSP_LED_BTN0_PORT, CYBSP_LED_BTN0_NUM, CYBSP_LED_STATE_OFF);
		Cy_GPIO_Write(CYBSP_LED_BTN1_PORT, CYBSP_LED_BTN1_NUM, CYBSP_LED_STATE_OFF);
		Cy_GPIO_Write(CYBSP_LED_BTN2_PORT, CYBSP_LED_BTN2_NUM, CYBSP_LED_STATE_OFF);
		Cy_TCPWM_TriggerStopOrKill(Timer_HW, Timer_MASK);

		//Cy_TCPWM_TriggerStart(PWM2_HW, (PWM2_MASK | PWM3_MASK));

		retVal = CY_SYSPM_SUCCESS;
		break;

	case CY_SYSPM_AFTER_TRANSITION:
		/* Add code here to execute after exiting sleep mode (back to run) */

		Cy_GPIO_Write(CYBSP_USER_LED_PORT, CYBSP_USER_LED_NUM, 1);
		Cy_TCPWM_TriggerReloadOrIndex(Timer_HW, Timer_MASK);

		//Cy_TCPWM_TriggerStopOrKill(PWM2_HW, (PWM2_MASK | PWM3_MASK));

		game_reset();

		retVal = CY_SYSPM_SUCCESS;
		break;

	default:
		break;
	}
	return (retVal);
}
/* [] END OF FILE */

