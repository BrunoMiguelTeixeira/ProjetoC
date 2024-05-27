/*--- SYSTEM CLOCKS ---*/
#define SYSCLK 80000000L        /**< System clock frequency, in Hz          */
#define PBCLOCK 40000000L       /**< Peripheral Bus clock frequency, in Hz  */

/* Analog to Digital section: */
#define ADC_CHAN_SCALE1 7       /**< ADC channel for the loadcell (0 to 15) */
#define ADC_CHAN_SCALE2 8       /**< ADC channel for the loadcell (0 to 15) */
#define SAMPL_FREQ_HZ 100        /**< Sampling frequency, in Hz              */

/* PWM section: */
#define PWM_OC_CHAN 1           /**< PWM output compare channel (1 to 5)    */
#define PWM_FREQ_HZ 2000        /**< PWM frequency, in Hz                   */