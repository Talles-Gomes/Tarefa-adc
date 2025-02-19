#include <stdio.h>
#include "pico/stdlib.h"
#include <stdlib.h>
#include "hardware/adc.h"     
#include "hardware/pwm.h"     
#include "hardware/i2c.h"
#include "inc/ssd1306.h"

#define LED_R 13 
#define LED_B 12 
#define LED_G 11  
#define margem 250

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

ssd1306_t oled;
const uint16_t WRAP_PERIOD = 62500; //valor máximo do contador - WRAP
bool cor = true;

int main() {
    
    stdio_init_all();

    adc_init(); 

    gpio_set_function(LED_B, GPIO_FUNC_PWM); //habilitar o pino como PWM
    uint slice = pwm_gpio_to_slice_num(LED_B); //obter o canal PWM
    pwm_set_wrap(slice, WRAP_PERIOD); //definir o valor de wrap
    pwm_set_enabled(slice, true); //habilita o pwm no slice correspondente

    gpio_set_function(LED_R, GPIO_FUNC_PWM); //habilitar o pino como PWM
    uint slice_led = pwm_gpio_to_slice_num(LED_R); //obter o canal PWM da GPIO
    pwm_set_wrap(slice_led, WRAP_PERIOD); //definir o valor de wrap
    pwm_set_enabled(slice_led, true); //habilita o pwm no slice correspondente

    i2c_init(I2C_PORT, 400 * 1000);  // Inicializa I2C a 400kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&oled, 128, 64, false, 0x3C, I2C_PORT);  // Inicializa display
    ssd1306_config(&oled); // Configura o display    

    int box_x = 60 ;
    int box_y = 28;

    while (true) {


        adc_select_input(1);  
        uint16_t vrx_value = adc_read(); 
        if (abs((int)vrx_value - 2048) > margem){
        pwm_set_gpio_level(LED_R, abs((int)vrx_value - 2048)); 
        }else
        pwm_set_gpio_level(LED_R, 0); 

        adc_select_input(0);  
        uint16_t vry_value = adc_read(); 
        if (abs((int)vry_value - 2048) > margem){
        pwm_set_gpio_level(LED_B, abs((int)vry_value - 2048)); 
        }else
        pwm_set_gpio_level(LED_B, 0);

        
        
     if (vrx_value/32 < 124 && vrx_value/32 > 1){
       box_x = (uint16_t)vrx_value/32 ;}
    if (vry_value/64 < 60 && vry_value/64 > 1){
        box_y = 64 - (uint16_t)vry_value/64;}

        // Atualiza display
        ssd1306_fill(&oled, !cor);
        ssd1306_rect(&oled, box_y-4, box_x-4, 8, 8,cor,cor);
        ssd1306_rect(&oled, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
        ssd1306_send_data(&oled);

        sleep_ms(100);  
    }

    return 0;  
}