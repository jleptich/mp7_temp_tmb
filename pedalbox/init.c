#include "pico/stdlib.h"
#include "hardware/irq.h"

//creating definations for all the pins that need to be set 
#define PWM 7       //used for the clock signal to both ADCS
#define START_ALE_ADC1 9    //Needs to be set high for at least 200ns to start the ADC
#define START_ALE_ADC2 0    //Needs to be set high for at least 200ns to start the ADC
#define ADDA 20     //first bit for the multiplexer selection
#define ADDB 19    //second bit for the multiplexer selection
#define ADDC 18    //third bit for the multiplexer selection
#define EOC_ADC1 12     //End of conversion pin for ADC1, goes high when the conversion is done
#define EOC_ADC2 1      //End of conversion pin for ADC2, goes high when the conversion is done

//Output values of the ADC in order of MSB to LSB
#define INPUT1 28
#define INPUT2 27
#define INPUT3 26
#define INPUT4 22
#define INPUT5 13
#define INPUT6 17
#define INPUT7 14
#define INPUT8 21

//CAN transciever and reciever pins
#define TXD 16
#define RXD 15

//output enable pins for both ADCs
#define EN1 11
#define EN2 2




void init_pins(){
    //setting up the PWM pin
    gpio_init(PWM);
    GPIO_SET_FUNCTION(PWM, GPIO_FUNC_PWM);
    gpio_set_dir(PWM, GPIO_OUT);
    uint slice = pwm_gpio_to_slice_num(PWM);        // = 3
    uint chan  = pwm_gpio_to_channel(PWM);          // = PWM_CHAN_B

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&cfg, 1);           // DIV = 1
    pwm_config_set_wrap(&cfg, 249);               // TOP = 249
    pwm_init(slice, &cfg, false);                 // load config, don't start yet

    pwm_set_chan_level(slice, chan, 125);         // CC_B = 125 → 50%
    pwm_set_enabled(slice, true);                 // go

    //Start ALE pins for the ADCS are setup here
    gpio_init(START_ALE_ADC1);
    gpio_set_dir(START_ALE_ADC1, GPIO_OUT);
    gpio_put(START_ALE_ADC1, 0);

    gpio_init(START_ALE_ADC2);
    gpio_set_dir(START_ALE_ADC2, GPIO_OUT);
    gpio_put(START_ALE_ADC2, 0);

    //multiplexer selection pins set up here
    gpio_init(ADDA);
    gpio_set_dir(ADDA, GPIO_OUT);
    gpio_put(ADDA, 0);

    gpio_init(ADDB);
    gpio_set_dir(ADDB, GPIO_OUT);
    gpio_put(ADDB, 0);

    gpio_init(ADDC);
    gpio_set_dir(ADDC, GPIO_OUT);
    gpio_put(ADDC, 0);

    //End of conversion pins set 
    gpio_init(EOC_ADC1);
    gpio_set_dir(EOC_ADC1, GPIO_IN);

    gpio_init(EOC_ADC2);
    gpio_set_dir(EOC_ADC2, GPIO_IN);

    //The ADC input pins are setup here
    gpio_init(INPUT1);
    gpio_set_dir(INPUT1, GPIO_IN);

    gpio_init(INPUT2);
    gpio_set_dir(INPUT2, GPIO_IN);

    gpio_init(INPUT3);
    gpio_set_dir(INPUT3, GPIO_IN);

    gpio_init(INPUT4);
    gpio_set_dir(INPUT4, GPIO_IN);

    gpio_init(INPUT5);
    gpio_set_dir(INPUT5, GPIO_IN);

    gpio_init(INPUT6);
    gpio_set_dir(INPUT6, GPIO_IN);

    gpio_init(INPUT7);
    gpio_set_dir(INPUT7, GPIO_IN);

    gpio_init(INPUT8);
    gpio_set_dir(INPUT8, GPIO_IN);

    gpio_init(EN1);
    gpio_set_dir(EN1, GPIO_IN);

    gpio_init(EN2);
    gpio_set_dir(EN2, GPIO_IN);

    //Can transcierver and reciever pin
    gpio_init(TXD);
    gpio_set_dir(TXD, GPIO_OUT);

    gpio_init(RXD);
    gpio_set_dir(RXD, GPIO_IN);
}


void select_adc_input(uint8_t input) {
    // Set the multiplexer selection pins based on the input number
    gpio_put(ADDA, (input & 0x01) ? 1 : 0);
    gpio_put(ADDB, (input & 0x02) ? 1 : 0);
    gpio_put(ADDC, (input & 0x04) ? 1 : 0);
}


void start_adc_conversion(uint8_t adc_number){
    if (adc_number == 1) {
        gpio_put(START_ALE_ADC1, 1);
        sleep_us(2); // Ensure the pulse is long enough
        gpio_put(START_ALE_ADC1, 0);
    } else if (adc_number == 2) {
        gpio_put(START_ALE_ADC2, 1);
        sleep_us(2); // Ensure the pulse is long enough
        gpio_put(START_ALE_ADC2, 0);
    }
}



