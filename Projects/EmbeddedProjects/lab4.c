//Lab 4- Train Station
//By Brodie Gerloff and Wallace Obey

//Uses timer interrupts and external LEDs to simulate a train station with warning lights and a stopping button


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "inc/tm4c123gh6pm.h"       //Library for interrupt/register assignments
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"    //Library for interrupts
#include "driverlib/gpio.h"         //Library for GPIO pins
#include "driverlib/timer.h"         //Library for timers


#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
    while(1);
}
#endif

//Global Variables
int32_t switchRead = 0;     //Result of reading pin PB5
uint8_t state = 1;          //Current state for FSM
uint8_t halfPoint = 0;      //Determines if the device is halfway between the
uint8_t forward = 1;

//Function to set all blue LEDS. if given 0, shuts of all blue LEDs
void led_set(uint8_t led);

//ISR for timer interrupt
void timer_isr(void);

int main(void) {

    //Variables for the timer interrupt
    uint32_t period;

    //Set clock speed to 50 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    // Enable GPIO port PB
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Check if the peripheral access is enabled.
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)){}

    //Pin setups. All are outputs except PB4 (unused) and PB5 (input)
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_6|GPIO_PIN_7); //0xBF
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_5);

    //Enable the timer, set delay for 1 second
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    period = SysCtlClockGet();
    TimerLoadSet(TIMER0_BASE, TIMER_A, period - 1);

    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntRegister(TIMER0_BASE, TIMER_A, timer_isr);
    IntMasterEnable();

    TimerEnable(TIMER0_BASE, TIMER_A);

    //Start with green led on
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_PIN_1);

    //Loop until interrupt
    while(1);

}

void timer_isr(void) {
    switch(state) {
        case 1:                                                     //UNCM
            led_set(GPIO_PIN_2);
            forward = 1;
            switchRead = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5);  //Reads switch
            if (switchRead < 5) {
                state = 2;
            }
            break;
        case 2:                                                     //UNCM - JWC
            led_set(0);
            if (halfPoint == 0) {       //Test if passed midpoint
                halfPoint = 1;
            }
            else {
                if (forward == 1) {     //Test if going forward, goto JWC
                    state = 3;
                    halfPoint = 0;
                }
                else {                  //Backward, UNCM
                    state = 1;
                    halfPoint = 0;
                }
            }
            break;
        case 3:                                                     //JWC
            led_set(GPIO_PIN_3);
            switchRead = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5);  //Test switch
            if (switchRead < 5) {
                if (forward == 1) {     //Test if going forward
                    state = 4;
                }
                else {
                    state = 2;
                }
            }
            break;
        case 4:                                                     //JWC - McC
            led_set(0);
            if (forward == 1) {     //Test if going forward
                if (halfPoint == 0) {
                    halfPoint = 1;
                }
                else {
                    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0);
                    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x0);
                    state = 5;
                    halfPoint = 0;
                }
            }
            else {
                if (halfPoint == 0) {
                    halfPoint = 1;
                }
                else {
                    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_PIN_1);
                    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0x0);
                    state = 3;
                    halfPoint = 0;
                }
            }
            break;
        case 5:                                                     //McC
            led_set(GPIO_PIN_7);
            switchRead = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5);
            if (switchRead < 5) {
                if (forward == 1) {     //Test if going forward
                    state = 6;
                }
                else {
                    state = 4;
                }
            }
            break;
        case 6:                                                     //McC - UCB
            led_set(0);
            if (forward == 1) {     //Test if going forward
                if (halfPoint == 0) {
                    halfPoint = 1;
                }
                else {
                    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_PIN_1);
                    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0x0);
                    state = 7;
                    halfPoint = 0;
                }
            }
            else {
                if (halfPoint == 0) {
                    halfPoint = 1;
                }
                else {
                    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0);
                    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x0);
                    state = 5;
                    halfPoint = 0;
                }
            }
            break;
        case 7:                                                     //UCB
            led_set(GPIO_PIN_6);
            forward = 0;
            switchRead = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5);
            if (switchRead < 5) {
                state = 6;
            }
            break;
    }

    TimerIntClear( TIMER0_BASE, TIMER_TIMA_TIMEOUT );
}

void led_set(uint8_t ledPin) {
    //Clear all leds
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_6|GPIO_PIN_7, 0);

    //Turn on the set pin, unless given 0.
    if (ledPin != 0) {
        GPIOPinWrite(GPIO_PORTB_BASE, ledPin, ledPin);
    }
}
