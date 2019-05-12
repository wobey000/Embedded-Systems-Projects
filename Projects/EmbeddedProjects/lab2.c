//Lab 2- Embedded Systems
//By Brodie Gerloff and Wallace Obey

//Creates a simple loop that tests for input to a switch to determine if a light blinks red or green

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "buttons.h"

//Debug code for driver library- copied from blinky code

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
    while(1);
}
#endif

uint8_t myDelay(void);
//Creates a 4 second delay and records the output state

void main() {

    uint8_t state = 0x00;           //State received from the delay function. Determines the next LED color.
    uint8_t prev = 0x10;            //Last state. Used if no button is pushed, or if an error occurs. Starts on red.

    //Set clock speed to 80MHz
    SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    // Enable GPIO port PF
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // Check if the peripheral access is enabled.
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)){}

    //Enable GPIO pins for the red and green LEDs
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);

    //Initialize Buttons
    ButtonsInit();

    //Setting SysTick
    HWREG(NVIC_ST_CTRL) = 0;                //Disable the control
    HWREG(NVIC_ST_RELOAD) = 16000000 - 1;   //set reload to 0.2 Seconds
    HWREG(NVIC_ST_CURRENT) = 0;             //Clear the current value
    HWREG(NVIC_ST_CTRL) |= 0x05;            //Enable systick with system clock

    while(true) {

        if (state == 0x00) {                                            //No input, repeat previous
            if (prev == 0x10) {
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x0);
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
            }
            else {
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x0);
            }
        }
        else if (state == 0x01) {                                       //Right button, blink green
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x0);
            prev = 0x01;
        }
        else if (state == 0x10) {                                       //Left button, blink red
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x0);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
            prev = 0x10;
        }
        else {                                                          //Error, either both buttons or hitting one button multiple times
            if (prev == 0x10) {
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x0);
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
            }
            else {
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x0);
            }
        }

        state = myDelay();

        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x0);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x0);

        state |= myDelay();

    }

}

uint8_t myDelay(void) {
    uint8_t i = 0;
    uint8_t state = 0;
    uint8_t delta = 0;
    uint8_t rawState = 0;
    uint8_t returnState = 0;

    for (i = 0; i < 20; i++) {
        while( (HWREG(NVIC_ST_CTRL) & (1<<16) ) == 0){}     //Checks COUNTFLAG
        state = ButtonsPoll(&delta, &rawState);
        if (BUTTON_PRESSED(LEFT_BUTTON, state, delta)) {
            returnState |= LEFT_BUTTON;
        }
        if (BUTTON_PRESSED(RIGHT_BUTTON, state, delta)) {
            returnState |= RIGHT_BUTTON;
        }
    }
    return returnState;
}
