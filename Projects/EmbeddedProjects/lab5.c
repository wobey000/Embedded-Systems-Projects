//Lab 5- Music Note Player
//Creates "Music" using a software created DAC.

//Brodie Gerloff and Wallace Obey

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>                  //Used for sine function
#include <math.h>
#include "inc/hw_memmap.h"
#include "inc/tm4c123gh6pm.h"       //Library for interrupt/register assignments
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"    //Library for interrupts
#include "driverlib/gpio.h"         //Library for GPIO pins
#include "driverlib/timer.h"        //Library for timers

//DEFINES
#define TWO_PI 6.2832               //2 * Pi, used for sine estimation
#define DEF_FREQ 10000               //Default ISR frequency

//FUNCTION DECLARATIONS
__interrupt void dac_isr(void);
__interrupt void timer_isr(void);

//GLOBAL VARIABLE DECLARATION
const uint16_t notes[] = {440, 494, 523, 587, 659, 699, 784, 0};
volatile uint8_t n = 0;             //Determines current note
volatile uint8_t output = 0;
volatile float angle = 0.0;

uint8_t counter = 0;                //Used for the 1 second timer

int main(void) {
    //Variable declarations
    uint32_t period = 0;

    //Set clock speed to 50 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    // Enable GPIO Port PB
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Check if the peripheral access is enabled.
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)){}

    //Pin setups. See
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    //Enable the DAC timer, set delay for 1/10,000 of a second
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    period = (SysCtlClockGet() / DEF_FREQ);
    TimerLoadSet(TIMER0_BASE, TIMER_A, period - 1);

    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntRegister(TIMER0_BASE, TIMER_A, dac_isr);

    TimerEnable(TIMER0_BASE, TIMER_A);

    //Enable the 1 second timer
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    period = SysCtlClockGet();
    TimerLoadSet(TIMER1_BASE, TIMER_A, period - 1);

    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntRegister(TIMER1_BASE, TIMER_A, timer_isr);
    IntMasterEnable();

    TimerEnable(TIMER1_BASE, TIMER_A);

    //Main loop
    while(1){
    }

}

__interrupt void dac_isr(void) {
    //set the angle, if > 2 pi reset
    angle += (TWO_PI * 2 * notes[n] / DEF_FREQ );
    if (angle >= TWO_PI) {
        angle = 0;
    }

    //Set the output
    output = round(7.5 + 7.5 * sin(angle));

    //Mask the output to the correct pins
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, (GPIO_PIN_0 & output));
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, (GPIO_PIN_1 & output));
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, (GPIO_PIN_2 & output));
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, (GPIO_PIN_3 & output));

    //Clear interrupt
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

__interrupt void timer_isr(void) {

    //Increment 1 second counter
    if (counter < 35) {
        counter++;
    }

    //Checks timing to pick note
    switch(counter) {
        case 8: //8,11,17,19,23,28
            n = 1;
            break;
        case 11:
            n = 2;
            break;
        case 17:
            n = 3;
            break;
        case 19:
            n = 4;
            break;
        case 23:
            n = 5;
            break;
        case 28:
            n = 6;
            break;
        case 29:
            n = 7;
            break;
        default:
            break;
    }


    //Clear interrupt
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
}
