#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"

uint32_t sysclock;

void main()
{
    float PWM_FREQ;
    float CPU_FREQ;
    float PWM_WORD;

    PWM_FREQ = 10000; //10khz
    CPU_FREQ = 120000000;
    PWM_WORD = (1/PWM_FREQ)*CPU_FREQ;
    sysclock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                                     SYSCTL_OSC_MAIN |
                                                     SYSCTL_USE_PLL |
                                                     SYSCTL_CFG_VCO_480), CPU_FREQ);
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1); // Enable clock to PWM module
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0); // Use PWM module 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG); // Enable Port G
    GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_1); // Enable alternate function of PG1
    GPIOPinConfigure(GPIO_PG1_M0PWM5);
    PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_DB_NO_SYNC);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, PWM_WORD); // Set PWM frequency
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_5, PWMGenPeriodGet(PWM0_BASE, PWM_GEN_2)/1000); // Set Duty cycle 
    PWMOutputState(PWM0_BASE, PWM_OUT_5_BIT, true); // Enable PWM output channel 5
    PWMGenEnable(PWM0_BASE, PWM_GEN_2); // Enable PWM module

    while(1);

}



