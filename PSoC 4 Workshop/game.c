/*
 * game.c
 *
 *  Created on: Oct 31, 2022
 *      Author: jarvisc
 */

#include "cy_pdl.h"
#include "cybsp.h"
#include <stdlib.h>


#define LVL1_LED_PORT  	P2_2_PORT
#define LVL1_LED_NUM   	P2_2_NUM
#define LVL2_LED_PORT  	P2_0_PORT
#define LVL2_LED_NUM   	P2_0_NUM
#define LVL3_LED_PORT  	P1_6_PORT
#define LVL3_LED_NUM   	P1_6_NUM

#define LVL0_PERIOD		1000
#define LVL1_PERIOD		800
#define LVL2_PERIOD		650
#define LVL3_PERIOD		500

#define LVL1_THRESHOLD	10
#define LVL2_THRESHOLD	20
#define LVL3_THRESHOLD	30



volatile int32_t score = -1;


void game_init(void)
{
	Cy_GPIO_Pin_FastInit(LVL1_LED_PORT, LVL1_LED_NUM, CY_GPIO_DM_STRONG_IN_OFF, CYBSP_LED_STATE_OFF, HSIOM_SEL_GPIO);
	Cy_GPIO_Pin_FastInit(LVL2_LED_PORT, LVL2_LED_NUM, CY_GPIO_DM_STRONG_IN_OFF, CYBSP_LED_STATE_OFF, HSIOM_SEL_GPIO);
	Cy_GPIO_Pin_FastInit(LVL3_LED_PORT, LVL3_LED_NUM, CY_GPIO_DM_STRONG_IN_OFF, CYBSP_LED_STATE_OFF, HSIOM_SEL_GPIO);

	Cy_TCPWM_Counter_SetPeriod(Timer_HW, Timer_NUM, LVL0_PERIOD);

	score = -1;
}

void game_advance(void)
{
	score++;

	if (score == LVL1_THRESHOLD) {
		Cy_SCB_UART_PutString(CYBSP_UART_HW, "Level 1 - Faster!\r\n");
		Cy_GPIO_Write(LVL1_LED_PORT, LVL1_LED_NUM, CYBSP_LED_STATE_ON);
		Cy_TCPWM_Counter_SetPeriod(Timer_HW, Timer_NUM, LVL1_PERIOD);
	} else if (score == LVL2_THRESHOLD) {
		Cy_SCB_UART_PutString(CYBSP_UART_HW, "Level 2 - Faster!\r\n");
		Cy_GPIO_Write(LVL2_LED_PORT, LVL2_LED_NUM, CYBSP_LED_STATE_ON);
		Cy_TCPWM_Counter_SetPeriod(Timer_HW, Timer_NUM, LVL2_PERIOD);
	} else if (score == LVL3_THRESHOLD) {
		Cy_SCB_UART_PutString(CYBSP_UART_HW, "Level 3 - Faster!\r\n");
		Cy_GPIO_Write(LVL3_LED_PORT, LVL3_LED_NUM, CYBSP_LED_STATE_ON);
		Cy_TCPWM_Counter_SetPeriod(Timer_HW, Timer_NUM, LVL3_PERIOD);
	}

}

void game_over(void)
{
	char buffer[8];

	itoa(score,buffer,10);
	Cy_SCB_UART_PutString(CYBSP_UART_HW, "Game Over - Final Score: ");
	Cy_SCB_UART_PutString(CYBSP_UART_HW, buffer);
	Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n");

	Cy_GPIO_Write(LVL1_LED_PORT, LVL1_LED_NUM, CYBSP_LED_STATE_OFF);
	Cy_GPIO_Write(LVL2_LED_PORT, LVL2_LED_NUM, CYBSP_LED_STATE_OFF);
	Cy_GPIO_Write(LVL3_LED_PORT, LVL3_LED_NUM, CYBSP_LED_STATE_OFF);


}

void game_reset(void)
{
	score = -1;

	Cy_TCPWM_Counter_SetPeriod(Timer_HW, Timer_NUM, LVL0_PERIOD);

}
