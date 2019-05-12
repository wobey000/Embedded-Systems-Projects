//Lab 3- Button interrupts
//By Brodie Gerloff

//Uses the interrupt generated from pressing a button to change an LED.


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"    //Library for interrupts
#include "driverlib/gpio.h"         //Library for GPIO pins

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
    while(1);
}
#endif

void gpio_isr(void);

enum ledState{red, green, blue} state = red;

int main(void) {

    //Set clock speed to 50 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    // Enable GPIO port PF
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // Check if the peripheral access is enabled.
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)){}

    //Enables output for all LEDs
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    //Set Up interrupt for SW1 (Pin PF4)
    GPIOIntDisable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);                                        //Disable interrupt
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);  //Pull up register setup
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);                                              //Clear interrupts
    GPIOIntRegister(GPIO_PORTF_BASE, gpio_isr);                                             //Set interrupt to gpio_isr
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);                         //Rising edge trigger
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);                                         //Enable interrupt
    IntMasterEnable();

    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);

    while(1);

}

void gpio_isr(void) {
    switch(state){
        case red:
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x0);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
            state = green;
            break;
        case green:
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x0);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
            state = blue;
            break;
        case blue:
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0x0);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
            state = red;
    }
    /* if(state == 0) {                                            //Last state red
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x0);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
        state = 1;
    }
    else if (state == 1) {                                      //Last state green
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x0);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
        state = 2;
    }
    else if (state == 2) {                                      //Last state blue
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0x0);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
        state = 0;
    }
    else {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0x0);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x0);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
        state = 0;
    } */

    SysCtlDelay(1000000);                                        //1/50 second delay to prevent multiple presses
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);                  //Clear the interrupt flag
}
