#include "config_bits.h"
#include "sys_config.h"
#include "uart.h"
#include "adc.h"
#include "timer.h"
#include "pwm.h"
#include "menu.h"
#include <sys/attribs.h>
#include <xc.h>
#include <stdio.h>


/* GLOBAL VARIABLES/DEFINES FOR THE INTERRUPTS */
// Defines for easy Button access (Instead of PORTxbits.Rx)
#define SCALES_BTN  PORTFbits.RF5       /**< Button to change the scales ([0 ..400]; [0 .. 800]; Automatic) PIN16*/
#define TARE_BTN    PORTFbits.RF4       /**< Button to 'reset' / tare the scale PIN17*/
#define MAX_BTN     PORTDbits.RD15      /**< Button to enter Max Mode, showing the highest value read PIN18*/
#define HOLD_BTN    PORTDbits.RD14      /**< Button to hold the current weight read PIN19*/

#define SCALE1_MODE 0                   /**< Scale 1 Mode (0-400g) */
#define SCALE2_MODE 1                   /**< Scale 2 Mode (0-800g) */
#define AUTO_MODE   2                   /**< Automatic Mode */

volatile uint8_t scaleFlag = 0;         /**< Flag to identify the scale choice */
volatile uint8_t tareFlag = 0;          /**< Flag to identify the tare choice */
volatile uint8_t maxFlag = 0;           /**< Flag to identify the max choice */
volatile uint8_t holdFlag = 0;          /**< Flag to identify the hold choice */

/* Read values from the buttons, clearing the interrupt for the CNISR*/
volatile uint8_t newScale = 0, newTare = 0, newMax = 0, newHold = 0;
volatile uint8_t oldScale = 0, oldTare = 0, oldMax = 0, oldHold = 0;

volatile uint16_t val = 0;               /**< Variable to hold the ADC value */
volatile uint16_t tareVal = 0;           /**< Variable to hold the tare value */
volatile uint16_t printVal = 0;          /**< Variable to hold the ADC value to print */

uint8_t scaleMode = 0;                  /**< Variable to hold the scale mode */

/* FUNCTION TO READ THE MICROCONTROLLER'S TIME */
/* 
uint32_t ReadCoreTimer(void){
    volatile uint32_t value;
    asm volatile("mfc0 %0,$9" : "=r"(value));
    return value;
} 
*/

/* FUNCTION TO DELAY IN MILLISECONDS */
void delay_ms(int ms){
    for(int i = 0; i < ms; i++){
        for(int j = 0; j < 1000; j++);
    }
}

/* INTERRUPT CALLBACK PWM, OUTPUT DESIRED DUTY-CYCLE  */
void __ISR (_TIMER_2_VECTOR, IPL5SOFT) T2Interrupt(void)
{

    ClearIntFlagTimer2();
}

/* INTERRUPT CALLBACK TIMER3, READ ADC value */
void __ISR (_TIMER_3_VECTOR, IPL5SOFT) T3Interrupt(void)
{
    // Read the value from the ADC (Thermocouple)
    val = 0;
    ADC_input(ADC_CHAN_SCALE);
    ADC_start();
    while(ADC_IF() == 0);
    val = ADC_read();
    
    // Conversion to Millivolts
    val = (val*3300) / 1023;

    IFS1bits.AD1IF = 0; // Reset interrupt ADC
    ClearIntFlagTimer3();
}

/* INTERRUPT CALLBACK, CHANGE NOTIFICATION (CN) */
void __ISR(_CHANGE_NOTICE_VECTOR, IPL6SOFT) CNInterrupt(void){
    /* Since the CN Interrupt occours each GPIO change,
     * a filter is needed to avoid the FALLING-EDGE state change
     */
    newScale = SCALES_BTN;
    newTare = TARE_BTN;
    newMax = MAX_BTN;
    newHold = HOLD_BTN;

    // If a rising edge is detected on the button, set the flags
    if(newScale && !oldScale){
        scaleFlag = 1;
    }
    else if(newTare && !oldTare){
        tareFlag = 1;
    }
    else if(newMax && !oldMax){
        maxFlag = 1;
    }
    else if(newHold && !oldHold){
        holdFlag = 1;
    }

    oldScale = newScale;
    oldTare = newTare;
    oldMax = newMax;
    oldHold = newHold;

    // Clear the interrupt flag
    IFS1bits.CNIF = 0;
}

int main(void){
    
    // Config LD5 (RC1) as output
    TRISCbits.TRISC1 = 0;       // Set LED5 as output
    LATCbits.LATC1 = 0;         // Turn off LED5

    /* --------------- SETUP CN INTERRUPTS --------------- */
    // Config Buttons as inputs 
    TRISFbits.TRISF5 = 1;       // Set BTN1 as input
    TRISFbits.TRISF4 = 1;       // Set BTN2 as input
    TRISDbits.TRISD15 = 1;      // Set BTN3 as input
    TRISDbits.TRISD14 = 1;      // Set BTN4 as input

    /* Read the ports to clear mismatches */
    oldScale = SCALES_BTN;
    oldTare = TARE_BTN;
    oldMax = MAX_BTN;
    oldHold = HOLD_BTN;

    __builtin_disable_interrupts(); // Disable all interrupts
    
    // Enable the CN module
    CNCONbits.ON = 1;             // Enable CN module
    
    // Enable CN for the buttons
    CNENbits.CNEN18 = 1;        // Enable CN for BTN1
    CNENbits.CNEN17 = 1;        // Enable CN for BTN2
    CNENbits.CNEN21 = 1;        // Enable CN for BTN3
    CNENbits.CNEN20 = 1;        // Enable CN for BTN4

    IPC6bits.CNIP = 6;          // Set CN interrupt priority
    IPC6bits.CNIS = 5;          // Set CN interrupt sub-priority  
    IFS1bits.CNIF = 0;          // Reset CN interrupt flag
    IEC1bits.CNIE = 1;          // Enable CN interrupt

    /* --------------- SETUP INTERRUPTS --------------- */
    /* Set Interrupt Controller for multi-vector mode */
    INTCONSET = _INTCON_MVEC_MASK;
    
    /* Enable Interrupt Exceptions */
    // set the CP0 status IE bit high to turn on interrupts globally
    __builtin_enable_interrupts();
    /* ------------------------------------------------ */

    /* ------------------ SETUP UART ------------------ */
    if(UartInit(PBCLOCK, 115200) != UART_SUCCESS) {
        PORTAbits.RA3 = 1; // If the LED is active, there was an error
        while(1);
    }
    __XC_UART = 1;  /* Redirect stdin/stdout/stderr to UART1*/
    /* ------------------------------------------------ */

    /* ------------------ SETUP ADC ------------------- */
    ADC_init();           
    ADC_input(ADC_CHAN_THERMO); // Set the input channel for the ADC       
    ADC_enable();
    /* ------------------------------------------------ */

    /* -------------- SETUP TIMERS & PWM -------------- */
    ConfigTimer2(PWM_FREQ_HZ, 0);   // Output 
    ConfigTimer3(SAMPL_FREQ_HZ);    // Input 
    ConfigTimer4(3, 1);             // Timer 4, 32 bit mode (Due to 1Hz frequency)

    StartTimer2();
    StartTimer3(); 
    StartTimer4();    
    
    //ConfigPWM(ocChannel,2,0);              // OCx, Timer2, 50% duty cycle
    //ConfigPWM(ocChannelControl,2,5);       // OCx, Timer2, 0% duty cycle
    /* ------------------------------------------------ */

    /* ------------------ VARIABLES ------------------- */            
    
    uint8_t userInput;              // Variable to hold the input integer.                
    int total = -1;
    uint8_t optionChoice = 1;       // Variable to identify if its to write a menu choice or value
    uint8_t desiredLoc;             // Variable to identify which allowed variable the user wants to alter.
    uint8_t k = 0;
    
    /* ------------------------------------------------ */


    PutStringn("Start!");
    delay_ms(100);  
    
    // In case of a reset, send the end of frame to the script
    PutStringn("downup");
    delay_ms(100);

    // Send the initial frame via UART, so the script can identify the start of the program
    // to start the data collection
    PutString("updown");


    while(1){        
        // Check the flags of the buttons and act accordingly
        if(scaleFlag){
            /* If the scaleFlag is set, increment the k variable
             * to change the scale mode. The scaleMode variable is used to
             * identify the scale mode (0-400g; 0-800g; Automatic)
             */
            scaleFlag = 0;
            scaleMode++;
            if(scaleMode == 3){
                scaleMode = 0;
            }
        }
        else if(tareFlag){
            /* If the tareFlag is set, then set the read value
             * as the new '0'
             */
            tareVal = val;
            tareFlag = 0;
        }
        else if(maxFlag){
            /* If the maxFlag is set, then hold the maximum value
             * read until the user presses the button again
             */
            maxFlag = 0;
        }
        else if(holdFlag){
            /* If the holdFlag is set, then hold the current value
             * read until the user presses the button again
             */
            holdFlag = 0;
        }

        // Timer 4 & 5 (32bit mode) to print the menu (Polling Method)
        if(GetIntFlagTimer5()){
            DefaultMenu(total_temp, minTempRead, maxTempRead);      // Print the default menu (Temp)
            Menu(choice, value, pi.kp, pi.ki);                // Print the menu with the choice and value 
            PORTCbits.RC1 = !PORTCbits.RC1;     // Toggle LED to see the timer's timing
            ClearIntFlagTimer5();
        }
        
        // Read Input User value
        userInput = GetInteger();
        switch(userInput){
            // 1-9 numbers for the user's choice
            case 0 ... 9:
                value = value*10 + userInput;
                break;

            // Enter key to confirm the choice
            case 10:
                total = value;
                break;
        };

        if(total != -1){
            //The user has entered a value; Check the menu to print
            if(optionChoice == 1){
                choice = total;
                total = -1;
                value = 0;
                optionChoice = 0;
            }else{
                switch(choice){
                    case 1:
                        break;
                    case 2:
                        break;
                    case 3:
                        break;
                    default:
                        break;
                };
                total = -1;
                value = 0;
                choice = 0;
                optionChoice = 1;
            }
        }     
    } 
    return 0;
}
