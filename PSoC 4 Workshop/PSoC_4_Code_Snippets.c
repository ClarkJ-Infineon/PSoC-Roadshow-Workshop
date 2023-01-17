
/****************************************************************************************************************************************
* Section 1 - Step 7
* Reference code for sending UART command to clear serial terminal screen
****************************************************************************************************************************************/
/* Code to "clear" serial terminal */
Cy_SCB_UART_PutString(CYBSP_UART_HW, "\x1b[2J\x1b[;H");



/****************************************************************************************************************************************
* Section 3 - Step 2
* Reference code to register an interrupt callback function initialized within main function
* Button Callbck function to be added to top of main.c file
****************************************************************************************************************************************/
/* Code to register a GPIO interrupt */
cy_stc_sysint_t ButtonIRQ_cfg = { .intrPriority = 3, .intrSrc = SW1_IRQ };
Cy_SysInt_Init(&ButtonIRQ_cfg, Button_Callback);
NVIC_EnableIRQ(SW1_IRQ);


/* Callback function for GPIO Interrupt */
void Button_Callback(void)
{
	Cy_GPIO_ClearInterrupt(SW1_PORT, SW1_NUM);
	Cy_GPIO_Inv(CYBSP_USER_LED1_PORT, CYBSP_USER_LED1_NUM);
}



/****************************************************************************************************************************************
* Section 4 - Step 2
* Reference code to for TCPWM Timer
****************************************************************************************************************************************/
/* Initialize and enable TCPWM Counter */
Cy_TCPWM_Counter_Init(Timer_HW, Timer_NUM, &Timer_config);
Cy_TCPWM_Counter_Enable(Timer_HW, Timer_NUM);

/* Register callback for Timer interrupt */
cy_stc_sysint_t TimerIRQ_cfg = { .intrPriority = 3, .intrSrc = Timer_IRQ };
Cy_SysInt_Init(&TimerIRQ_cfg, Timer_Callback);
NVIC_EnableIRQ(Timer_IRQ);

/* Start TCPWM Timer */
Cy_TCPWM_TriggerStart(Timer_HW, Timer_MASK);



/****************************************************************************************************************************************
* Section 4 - Step 3
* Callback function for timer interrupt with global flag
****************************************************************************************************************************************/
volatile bool timer_expired = false;
void Timer_Callback(void)
{
	Cy_TCPWM_ClearInterrupt(Timer_HW, Timer_NUM, CY_TCPWM_INT_ON_TC);
	timer_expired = true;
}



/****************************************************************************************************************************************
* Section 4 - Step 4
* Callback function for timer interrupt with global flag
****************************************************************************************************************************************/
for(;;)
{
	if (timer_expired) {
		timer_expired = false;
		Cy_GPIO_Inv(CYBSP_USER_LED1_PORT, CYBSP_USER_LED1_NUM);
	}
}


/****************************************************************************************************************************************
* Section 6 - Step 3
* Reference code to include generated code from CAPSENSE Configurator
****************************************************************************************************************************************/
#include "cycfg_capsense.h"



/****************************************************************************************************************************************
* Section 7 - Step 2
* Reference code to scan and process scan results of the configured CAPSENSE widgets
****************************************************************************************************************************************/
/* Scan and Process CAPSENSE widgets */
if (CY_CAPSENSE_NOT_BUSY == Cy_CapSense_IsBusy(&cy_capsense_context)) {
	if (CY_CAPSENSE_STATUS_SUCCESS == Cy_CapSense_ProcessAllWidgets(&cy_capsense_context) ) {
		process_touch();
	}
	Cy_CapSense_ScanAllWidgets(&cy_capsense_context);
}



/****************************************************************************************************************************************
* Section 7 - Step 4
* Reference code to process touch events for Button 0
* Code to be used as a template for Button 1 and Button 2
****************************************************************************************************************************************/
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
			Cy_GPIO_Write(CYBSP_LED_BTN0_PORT, CYBSP_LED_BTN0_NUM, CYBSP_LED_STATE_ON);
			// Game Over - Wrong Button
		}
	}
	prev_button0 = button0;

}



/****************************************************************************************************************************************
* Section 8 - Step 2
* Reference code to check for End of Game condition (all LEDs are not cleared)
* Reference code when incoorect button is pressed
****************************************************************************************************************************************/
/* IF statement to check if any CYBSP_LED_BTNx is still on */
if ( (Cy_GPIO_Read(CYBSP_LED_BTN0_PORT, CYBSP_LED_BTN0_NUM) == CYBSP_LED_STATE_ON) || \
     (Cy_GPIO_Read(CYBSP_LED_BTN1_PORT, CYBSP_LED_BTN1_NUM) == CYBSP_LED_STATE_ON) || \
	 (Cy_GPIO_Read(CYBSP_LED_BTN2_PORT, CYBSP_LED_BTN2_NUM) == CYBSP_LED_STATE_ON) )
{
	// Game Over - Too Slow
	Cy_SCB_UART_PutString(CYBSP_UART_HW, "Game Over - Too Slow\r\n");
	Cy_SysPm_CpuEnterSleep();
} else {
	// Good Job - Start next round

}


/* End of game scenario for incoorection button (pressed with LED is not lit) */
Cy_SCB_UART_PutString(CYBSP_UART_HW, "Game Over - Wrong Button\r\n");
Cy_SysPm_CpuEnterSleep();



/****************************************************************************************************************************************
* Section 8 - Step 3
* Reference function to generate a random number using TRNG
* Reference code to turn on specific LED based on random number
****************************************************************************************************************************************/
/* Unsigned Int for randomly generated number */
uint32_t rndNum = 0;


/* Enable the Crypto IP */
Cy_Crypto_Enable(CRYPTO);


/* Generate a 32-bit random number */
Cy_Crypto_Trng(CRYPTO, 32UL, &rndNum);


/* Code to turn on CYBSP_LED_BTNx corresponding to randomly generated number */
if (rndNum%3 == 0) {
	Cy_GPIO_Write(CYBSP_LED_BTN0_PORT, CYBSP_LED_BTN0_NUM, CYBSP_LED_STATE_ON);
} else if (rndNum%3 == 1) {
	Cy_GPIO_Write(CYBSP_LED_BTN1_PORT, CYBSP_LED_BTN1_NUM, CYBSP_LED_STATE_ON);
} else {
	Cy_GPIO_Write(CYBSP_LED_BTN2_PORT, CYBSP_LED_BTN2_NUM, CYBSP_LED_STATE_ON);
}



/****************************************************************************************************************************************
* Section 9 - Step 2
* Reference code to define low power callback function
****************************************************************************************************************************************/
/* Function prototype for Sleep_Callback function */
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



/****************************************************************************************************************************************
* Section 9 - Step 3
* Reference code to register low power callback function
****************************************************************************************************************************************/
/* Register Callback */
Cy_SysPm_RegisterCallback(&sleep_cb);



/****************************************************************************************************************************************
* Section 9 - Step 4
* Reference code for Sleep callback function
****************************************************************************************************************************************/
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

		retVal = CY_SYSPM_SUCCESS;
		break;

	case CY_SYSPM_AFTER_TRANSITION:
		/* Add code here to execute after exiting sleep mode (back to run) */

		Cy_GPIO_Write(CYBSP_USER_LED_PORT, CYBSP_USER_LED_NUM, 1);
		Cy_TCPWM_TriggerReloadOrIndex(Timer_HW, Timer_MASK);

		retVal = CY_SYSPM_SUCCESS;
		break;

	default:
		break;
	}
	return (retVal);
}



/****************************************************************************************************************************************
* Section 11 - Step 10
* Reference code for initializing Breathing LED (SMART I/O and PWMs)
****************************************************************************************************************************************/
/* Initialize and Enable Smart IO */
Cy_SmartIO_Init(BREATHING_HW, &BREATHING_config);
Cy_SmartIO_Enable(BREATHING_HW);

/* Initialize and Enable PWM signals */
Cy_TCPWM_PWM_Init(PWM2_HW, PWM2_NUM, &PWM2_config);
Cy_TCPWM_PWM_Enable(PWM2_HW, PWM2_NUM);

Cy_TCPWM_PWM_Init(PWM3_HW, PWM3_NUM, &PWM3_config);
Cy_TCPWM_PWM_Enable(PWM3_HW, PWM3_NUM);


/****************************************************************************************************************************************
* Section 11 - Step 11
* Reference code for Starting and Stopping PWM signals
****************************************************************************************************************************************/
/* Start PWM signals - Add to CY_SYSPM_BEFORE_TRANSITION */
Cy_TCPWM_TriggerStart(PWM2_HW, (PWM2_MASK | PWM3_MASK));

/* Stop PWM signals - Add to CY_SYSPM_AFTER_TRANSITION */
Cy_TCPWM_TriggerStopOrKill(PWM2_HW, (PWM2_MASK | PWM3_MASK));
