#include "csc_servos.h"

#include "LPC21xx.h"
#include "std.h"
#include "sys_time.h"

#define CSC_SERVOS_NB 4
#define CSC_SERVOS_PERIOD (2 * SYS_TICS_OF_SEC((1./250.))); /* 250 Hz */
//#define CSC_SERVOS_PERIOD 375000

#define PWM_SERVO_0_PINSEL PINSEL0 
#define PWM_SERVO_0_PINSEL_VAL 2
#define PWM_SERVO_0_PINSEL_BIT 14
#define PWM_SERVO_0_ENA PWMPCR_ENA2
#define PWM_SERVO_0_LATCH PWMLER_LATCH2

#define PWM_SERVO_1_PINSEL PINSEL0 
#define PWM_SERVO_1_PINSEL_VAL 2
#define PWM_SERVO_1_PINSEL_BIT 16
#define PWM_SERVO_1_ENA PWMPCR_ENA4
#define PWM_SERVO_1_LATCH PWMLER_LATCH4

#define PWM_SERVO_2_PINSEL PINSEL1 
#define PWM_SERVO_2_PINSEL_VAL 1
#define PWM_SERVO_2_PINSEL_BIT 10
#define PWM_SERVO_2_ENA PWMPCR_ENA5
#define PWM_SERVO_2_LATCH PWMLER_LATCH5

#define PWM_SERVO_3_PINSEL PINSEL0 
#define PWM_SERVO_3_PINSEL_VAL 2
#define PWM_SERVO_3_PINSEL_BIT 18
#define PWM_SERVO_3_ENA PWMPCR_ENA6
#define PWM_SERVO_3_LATCH PWMLER_LATCH6

#define PWM_SERVO_4_PINSEL PINSEL0 
#define PWM_SERVO_4_PINSEL_VAL 2
#define PWM_SERVO_4_PINSEL_BIT 0
#define PWM_SERVO_4_ENA PWMPCR_ENA1
#define PWM_SERVO_4_LATCH PWMLER_LATCH1

#define PWM_SERVO_5_PINSEL PINSEL0 
#define PWM_SERVO_5_PINSEL_VAL 2
#define PWM_SERVO_5_PINSEL_BIT 2
#define PWM_SERVO_5_ENA PWMPCR_ENA3
#define PWM_SERVO_5_LATCH PWMLER_LATCH3


#define SERVO_REG_0 PWMMR2
#define SERVO_REG_1 PWMMR4
#define SERVO_REG_2 PWMMR5
#define SERVO_REG_3 PWMMR6
#define SERVO_REG_4 PWMMR1
#define SERVO_REG_5 PWMMR3


int32_t csc_servo_val[CSC_SERVOS_NB];


void csc_servos_init(void) {
  
  /* set pins as PWM outputs */
#ifndef CSC_FRIED_CHIP
  PWM_SERVO_0_PINSEL |= (PWM_SERVO_0_PINSEL_VAL << PWM_SERVO_0_PINSEL_BIT);
#else
  PWM_SERVO_4_PINSEL |= (PWM_SERVO_4_PINSEL_VAL << PWM_SERVO_4_PINSEL_BIT);
  PWM_SERVO_5_PINSEL |= (PWM_SERVO_5_PINSEL_VAL << PWM_SERVO_5_PINSEL_BIT);
#endif
  //  PWM_SERVO_1_PINSEL |= (PWM_SERVO_1_PINSEL_VAL << PWM_SERVO_1_PINSEL_BIT);
  PWM_SERVO_2_PINSEL |= (PWM_SERVO_2_PINSEL_VAL << PWM_SERVO_2_PINSEL_BIT);
  //  PWM_SERVO_3_PINSEL |= (PWM_SERVO_3_PINSEL_VAL << PWM_SERVO_3_PINSEL_BIT);

  /* set servo refresh rate */
  PWMMR0 = CSC_SERVOS_PERIOD;
  /* enable PWM outputs in single edge mode*/
  PWMPCR = PWM_SERVO_0_ENA | PWM_SERVO_1_ENA | PWM_SERVO_2_ENA | PWM_SERVO_3_ENA
#ifdef CSC_FRIED_CHIP
 | PWM_SERVO_4_ENA | PWM_SERVO_5_ENA
#endif
;
  /* commit PWMMRx changes */
  PWMLER = PWMLER_LATCH0;
  /* enable PWM timer in PWM mode */
  PWMTCR = PWMTCR_COUNTER_ENABLE | PWMTCR_PWM_ENABLE;

}

void csc_servos_set(int32_t* val) {
  
  int i;
  for (i=0; i<CSC_SERVOS_NB; i++)
    csc_servo_val[i] = val[i];
  
  SERVO_REG_0 = val[0];
  SERVO_REG_1 = val[1];
  SERVO_REG_2 = val[2];
  SERVO_REG_3 = val[3];
#ifdef CSC_FRIED_CHIP
  SERVO_REG_4 = val[0];
  SERVO_REG_5 = val[2];
#endif

  PWMLER = PWM_SERVO_0_LATCH | PWM_SERVO_1_LATCH | PWM_SERVO_2_LATCH | PWM_SERVO_3_LATCH
#ifdef CSC_FRIED_CHIP
    | PWM_SERVO_4_LATCH | PWM_SERVO_5_LATCH
#endif 
    ;
}
