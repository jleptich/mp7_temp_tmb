#ifndef INIT_H
#define INIT_H


#ifdef __cplusplus
extern "C" {
#endif
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
typedef struct {
    float voltage;
    float temperature;
} VoltageTemperature;

void init_pins();
void select_adc_input(uint8_t input);
void start_adc_conversion(uint8_t adc_number);

#ifdef __cplusplus
}
#endif

#endif
