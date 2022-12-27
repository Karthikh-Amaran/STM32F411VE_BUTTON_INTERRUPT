
/*
 *  THIS IS PERFECTLY WORKING CODE WITH PROPER STEPS
 *
 *  This works only when the switch is Connected to GND
 *  LED +VE CONNECTED TO PD12
 *  LED -VE TO GND
 *
 *  SWITCH END1 CONNECTED TO GND
 *  SWITCH END2 CONNECTED TO PD5
 *
 *	FOR ANALYZING THE SWITCH SIGNAL USING LOGIC ANALYSER BY CONNECTING 2 CHANNELS OF LOGIC ANALYSER
 *	TO SWITCH END1 AND SWITCH END2
 *
 *	Here Some times the LED Would be Toggled Twice for One button press (Mostly during Long Press or Incomplete press)
 *	that is only due to the Corruption in the button press
 *	This can be clearly analyzed using the Logic Analyzer
 */

/**
 ******************************************************************************

 Configuring and Testing an Button Interrupt from scratch:

 Steps to be followed:
 	//0. Enable the Clocks for all the required peripherals
 	//1. Configure the GPIO Port Selection in SYSCFG_EXTICR
 	//2. Enable the EXTI Interrupt delivery using IMR
	//3. Configure both FTSR and RTSR
	//4. Program ISERx Registers based on the IRQ Number
	//5. Now Program inside the Suitable interrupt Handler
	//5. Program Inside the IRQ handler for clearing the EXTI_Pending Register, which was set automatically when the interrupt arrived


 Project: Configure the PORTD NO 5 to get the Interrupt at rising edge

 ******************************************************************************
 */

#define EXTI_BASEADDR            (0x40013C00U)
#define GPIOD_BASEADDR 			 (0x40020c00U)
#define RCC_BASEADDR 			 (0x40023800U)
#define SYSCFG_BASEADDR 		 (0x40013800U)



#include <stdint.h>

void delay(void)
{
	for(volatile uint32_t i=0;i<500000 ;i++);
}

int main(void)
{
	//0.
	// RCC Should be enabled for SYSCFG for using interrupts
	uint32_t *pRCC_APB2ENR = (uint32_t *)(RCC_BASEADDR+0x44);
	*pRCC_APB2ENR |= (0x01<<14);

	// RCC should be enabled for GPIOS for using them
	uint32_t *pRCC_AHB1ENR = (uint32_t *)(RCC_BASEADDR+0x30);
	*pRCC_AHB1ENR |= (0x01<<3);


	// To enable the EXTI:
	//1. Configure the GPIO Port Selection in SYSCFG_EXTICR
	/*
	 * Here the SYSCFG_EXTICRx is used for Configure or basically control the EXTI pin
	 * In this case we are going to get the Interrupt on PD5, in STM32 Boards GPIO Interrupts
	 * are given to NVIC via an EXTI Registers so we need to enable the Suitable EXTIx here EXTI5 -> 5 due to PD*5*
	 * But setting 0011 at EXTI 5 We are making/enabling the EXTI5 to get the interrupt when occurs
	 * 0011 is FOR PD, PA takes 0000, PB takes 0001 ..... abnd so on.
	 *
	 */

	volatile uint32_t *pSYSCFG_EXTICR2 = (uint32_t *)(SYSCFG_BASEADDR+0x0c);
	*pSYSCFG_EXTICR2 &= ~(0x3<<4); // Clearing to avoid Garbage values
	*pSYSCFG_EXTICR2 |= (0x3<<4); // Setting 0b0011 in EXTI5 to enable the EXTI for PD5

	// Now Configuring the EXTI:
 	//2. Enable the EXTI Interrupt delivery using IMR

	volatile uint32_t *pEXTI_IMR = (uint32_t *)(EXTI_BASEADDR+0x00);
	*pEXTI_IMR |=  (1<<5);

	//3. Configure both FTSR and RTSR
	volatile uint32_t *pEXTI_FTSR = (uint32_t *)(EXTI_BASEADDR+0x0c);
	*pEXTI_FTSR |= (1<<5);

	//4. ENABLE the ISERx Registers based on the IRQ Number (For the Processor Side)
	volatile uint32_t *pNVIC_ISER =(uint32_t *)(0xE000E100);
	/*
	 *    Here the IRQ Number for EXTI_Interrupts 9 to 5 is 23
	 *    So the 23rd bit of the NVIC_ISERx has to be enabled
	 *    Each NVIC_ISERx is 32 bits and 23 comes under NVIC_ISER0
	 *    So Setting the 23rd bit of NVIC_ISER0.
	 */
	*pNVIC_ISER |= (1<<23);

	/*
	 *  So here everything is set so when ever the Rising edge occurs in the PD5 Button
	 *  the EXTI_PENDING REGISTER for the PD5 Would be set (Automatically)
	 *  then the control will jump to the interrupt Handler
	 *  So now we have to program the Interrupt Handler
	 *
	 */
	delay();
}


void EXTI9_5_IRQHandler(void)
{
	delay();

	//5. Program Inside the IRQ handler for clearing the Interrupt request by setting ICERx Registers
	volatile uint32_t *pMODER = (uint32_t *) (GPIOD_BASEADDR+0x00);
	*pMODER |= 0x1<<24;
	volatile uint32_t *pODR = (uint32_t *) (GPIOD_BASEADDR+0x14);
	*pODR ^= 0x1<<12;

	// 7. Should also clear the Pending registers of EXTI
	// Once we press a button (i.e the interrupt triggered ) the pending bit of the EXTI is set
	// So after executing the interrupt we should clear the pending bit
	volatile uint32_t *pEXTI_IPR = (uint32_t *)(EXTI_BASEADDR+0x14);
	*pEXTI_IPR |= (1<<5);
}
