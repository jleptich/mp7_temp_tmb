#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "can.hpp"
#include "init.h"
#include "hardware/watchdog.h"
#include "hardware/timer.h"
#include <cstdint>


// DEFAULT


temp_Lookup_Table vt_Lookup[33] = {
		{2.44, -40},
		{2.42, -35},
		{2.40, -30},
		{2.38, -25},
		{2.35, -20},
		{2.32, -15},
		{2.27, -10},
		{2.23, -5},
		{2.17, 0},
		{2.11, 5},
		{2.05, 10},
		{1.99, 15},
		{1.92, 20},
		{1.86, 25},
		{1.80, 30},
		{1.74, 35},
		{1.68, 40},
		{1.63, 45},
		{1.59, 50},
		{1.55, 55},
		{1.51, 60},
		{1.48, 65},
		{1.45, 70},
		{1.43, 75},
		{1.40, 80},
		{1.38, 85},
		{1.37, 90},
		{1.35, 95},
		{1.34, 100},
		{1.33, 105},
		{1.32, 110},
		{1.31, 115},
		{1.30, 120}
};

void can_call(float average_temperature, float max_temperature, float min_temperature) {
    // Transmit a CAN message with the current temperature
    sCAN_Header header = {
        .priority = 0,
        .module = THERMO_CONTROL_BOARD,
        .direction = FROM,
        .command = 1, // temperature command
    };
    uint32_t id = header2id(header);
    msg.id = id;
    msg.dlc = 8; // 8 bytes
    msg.data[0] = 0x00;
    msg.data[1] = min_temperature;
    msg.data[2] = max_temperature;
    msg.data[3] = average_temperature;
    msg.data[4] = 0x28;
    msg.data[5] = 0x54;
    msg.data[6] = 0x01;
    msg.data[7] = msg.data[0] + msg.data[1] + msg.data[2] + msg.data[3] + msg.data[4] + msg.data[5] + msg.data[6] + 0x39 + 0x08;
    can2040_transmit(&cbus, &msg);
}

volatile bool adc1_ready = false;
volatile bool adc2_ready = false;

void my_bank0_isr(void){
    if(gpio_get_irq_event_mask(EOC_ADC1) & GPIO_IRQ_EDGE_RISE){
        gpio_acknowledge_irq(EOC_ADC1, GPIO_IRQ_EDGE_RISE);
        gpio_put(EN1, 1);
        adc1_ready = true;
    }
    else if(gpio_get_irq_event_mask(EOC_ADC2) & GPIO_IRQ_EDGE_RISE){
        gpio_acknowledge_irq(EOC_ADC2, GPIO_IRQ_EDGE_RISE);
        gpio_put(EN2, 1);
        adc2_ready = true;
    }
}




int main(){
    stdio_init_all();
    init_pins();
    
    
    irq_set_exclusive_handler(IO_IRQ_BANK0, my_bank0_isr);

    irq_set_enabled(IO_IRQ_BANK0, true);

    int adc_selection = 1;  // 1 ADC1 run, 2 ADC2 run 
    float voltage = 0;
    float total_temperature = 0;
    float average_temperature = 0;
    float min_temperature = 9999;
    unsigned int temperature_count = 0;
    float max_temperature = 0;
    uint8_t multiplexer_index = 0; 
    bool active1 = false;
    bool active2 = false;
    if(watchdog_caused_reboot()){
    adc_selection = 1;  // 1 ADC1 run, 2 ADC2 run 
    voltage = 0;
    total_temperature = 0;
    average_temperature = 0;
    min_temperature = 9999;
    temperature_count = 0;
    max_temperature = 0;
    multiplexer_index = 0;
    }
    watchdog_enable(500, 1); // Enable watchdog with a 5-second timeout and pause on debug
    while(true){
        if(adc_selection == 1){
            if(adc1_ready == false && active1 == false){
                select_adc_input(multiplexer_index);
                start_adc_conversion(1);
                active1 = true;
                sleep_ms(1); // Wait for ADC conversion to complete
            }
            else if(adc1_ready == true && active1 == true){
                uint8_t adc1_value = (gpio_get(INPUT1) << 7) | (gpio_get(INPUT2) << 6) | (gpio_get(INPUT3) << 5) | (gpio_get(INPUT4) << 4) |
                  (gpio_get(INPUT5) << 3) | (gpio_get(INPUT6) << 2) | (gpio_get(INPUT7) << 1) | gpio_get(INPUT8);
                voltage = (adc1_value * 3.3) / 255;
                for(int i = 0; i < 33; i++){
                    if(voltage >= vt_Lookup[i].voltage){
                        total_temperature += vt_Lookup[i].temperature;
                        temperature_count++;
                        if(vt_Lookup[i].temperature > max_temperature){
                            max_temperature = vt_Lookup[i].temperature;
                        }
                        if(vt_Lookup[i].temperature < min_temperature){
                            min_temperature = vt_Lookup[i].temperature;
                        }
                        break;
                    }
                }
                average_temperature = temperature_count > 0 ? total_temperature / temperature_count : 0;
                adc1_ready = false;
                can_call(average_temperature, max_temperature, min_temperature);
                watchdog_update(); // Reset the watchdog timer
                active1 = false;
                adc_selection = 2;
                gpio_put(EN1, 0); // Disable ADC1
            }
        }
        else if(adc_selection == 2){
            if(adc2_ready == false && active2 == false){
                select_adc_input(multiplexer_index);
                start_adc_conversion(2);
                active2 = true;
                sleep_ms(1); // Wait for ADC conversion to complete
            }
            else if(adc2_ready == true && active2 == true){
                uint8_t adc2_value = (gpio_get(INPUT1) << 7) | (gpio_get(INPUT2) << 6) | (gpio_get(INPUT3) << 5) | (gpio_get(INPUT4) << 4) |
                  (gpio_get(INPUT5) << 3) | (gpio_get(INPUT6) << 2) | (gpio_get(INPUT7) << 1) | gpio_get(INPUT8);
                voltage = (adc2_value * 3.3) / 255;
                for(int i = 0; i < 33; i++){
                    if(voltage >= vt_Lookup[i].voltage){
                        total_temperature += vt_Lookup[i].temperature;
                        temperature_count++;
                        if(vt_Lookup[i].temperature > max_temperature){
                            max_temperature = vt_Lookup[i].temperature;
                        }
                        if(vt_Lookup[i].temperature < min_temperature){
                            min_temperature = vt_Lookup[i].temperature;
                        }
                        break;
                    }
                }
                average_temperature = temperature_count > 0 ? total_temperature / temperature_count : 0;
                adc2_ready = false;
                active2 = false;
                adc_selection = 1;
                multiplexer_index = (multiplexer_index + 1) % 5; // Cycle through multiplexer inputs
                can_call(average_temperature, max_temperature, min_temperature);
                watchdog_update(); // Reset the watchdog timer
                gpio_put(EN2, 0); // Disable ADC2
            }
        }
    }

}
