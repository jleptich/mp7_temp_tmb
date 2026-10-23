#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "can.hpp"
#include "init.h"
#include "hardware/timer.h"
#include <cstdint>


// DEFAULT
// NOTE: these need tuned experimentally
static uint16_t min = 670;
static uint16_t max = 880;

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

bool can_tx_timer_callback(__unused struct repeating_timer *t) {
    ready_timer_cb();
    gpio_xor_mask(1 << 25);

    /* sample pots */
    adc_select_input(0);
    uint16_t pot0_taps = adc_read();
    adc_select_input(1);
    uint16_t pot1_taps = 4095 - adc_read();

    uint16_t diff = (pot0_taps > pot1_taps ? pot0_taps - pot1_taps : pot1_taps - pot0_taps);

    uint16_t pot_avg = diff < 200 ? (pot0_taps + pot1_taps)/2 : 0;

    double pot_avg_pct = ((static_cast<double>(pot0_taps) - min)/(max - min));
    uint16_t pos_taps = static_cast<uint16_t>(pot_avg_pct*4095.0);

    printf("pos_taps=%u bytes=%02X %02X\n", pos_taps, pos_taps & 0xFF, (pos_taps >> 8) & 0xFF);
    can_tx_adc_taps(pos_taps);
    //throttle_watchdog_reset();
    return true;
}

volatile uint32_t adc1_value;
volatile bool adc1_ready = false;
gpio_set_irq_enabled_with_callback(EOC_ADC1, GPIO_IRQ_EDGE_TO_ACTIVE, true, adc1_callback){
    adc1_value = (gpio_get(INPUT1) << 7) | (gpio_get(INPUT2) << 6) | (gpio_get(INPUT3) << 5) | (gpio_get(INPUT4) << 4) |
                  (gpio_get(INPUT5) << 3) | (gpio_get(INPUT6) << 2) | (gpio_get(INPUT7) << 1) | gpio_get(INPUT8);
    adc1_ready = true;
}

volatile uint32_t adc2_value;
volatile bool adc2_ready = false;
gpio_set_irq_enabled_with_callback(EOC_ADC2, GPIO_IRQ_EDGE_TO_ACTIVE, true, adc2_callback){
    adc2_value = (gpio_get(INPUT1) << 7) | (gpio_get(INPUT2) << 6) | (gpio_get(INPUT3) << 5) | (gpio_get(INPUT4) << 4) |
                  (gpio_get(INPUT5) << 3) | (gpio_get(INPUT6) << 2) | (gpio_get(INPUT7) << 1) | gpio_get(INPUT8);
    adc2_ready = true;
}




int main() {
    stdio_init_all();
    init_pins();
    int adc_selection = 1;  // 1 ADC1 run, 2 ADC2 run 
    float voltage = 0;
    float total_temperature = 0;
    float average_temperature = 0;
    unsigned int temperature_count = 0;
    float max_temperature = 0;
    uint8_t multiplexer_index = 0; 
    while(true){
        if(adc_selection == 1){
            if(adc1_ready == false){
                select_adc_input(multiplexer_index);
                start_adc_conversion(1);
            }
            else{
                voltage = (adc1_value * 3.3) / 4095;
                for(int i = 0; i < 33; i++){
                    if(voltage >= vt_Lookup[i].voltage){
                        total_temperature += vt_Lookup[i].temperature;
                        temperature_count++;
                        if(vt_Lookup[i].temperature > max_temperature){
                            max_temperature = vt_Lookup[i].temperature;
                        }
                        break;
                    }
                }
                average_temperature = temperature_count > 0 ? total_temperature / temperature_count : 0;
                adc1_ready = false;
                adc_selection = 2;
            }
        }
        else if(adc_selection == 2){
            if(adc2_ready == false){
                select_adc_input(multiplexer_index);
                start_adc_conversion(2);
            }
            else{
                voltage = (adc2_value * 3.3) / 4095;
                for(int i = 0; i < 33; i++){
                    if(voltage >= vt_Lookup[i].voltage){
                        total_temperature += vt_Lookup[i].temperature;
                        temperature_count++;
                        if(vt_Lookup[i].temperature > max_temperature){
                            max_temperature = vt_Lookup[i].temperature;
                        }
                        break;
                    }
                }
                average_temperature = temperature_count > 0 ? total_temperature / temperature_count : 0;
                adc2_ready = false;
                adc_selection = 1;
                multiplexer_index = (multiplexer_index + 1) % 5; // Cycle through multiplexer inputs
            }
        }
    }

}
