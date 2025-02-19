#include <stdio.h>
#include "pico/stdlib.h"
#include <stdlib.h>
#include "hardware/adc.h"     
#include "hardware/pwm.h"     
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "hardware/timer.h"

#define LED_R 13 
#define LED_B 12 
#define LED_G 11  
#define margem 200
#define botA 5
#define botJ 22

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
bool hab = true;
uint16_t vrx_value = 2048;
uint16_t vry_value = 2048;
int retx = 122 ;
int rety = 58;
int larg = 3;
int a=0;

static void gpio_irq_handler(uint gpio,uint32_t events);
static volatile uint32_t last_time = 0;

void gpio_irq_handler(uint gpio,uint32_t events)
    {
        uint32_t current_time = to_us_since_boot(get_absolute_time());
        if(current_time - last_time > 200000) // 200 ms de debouncing
        {
            last_time = current_time;
            if(gpio == 5){
                hab = !hab;
                pwm_set_gpio_level(LED_B, 0);
                pwm_set_gpio_level(LED_R, 0);
            }if (gpio == 22){
                gpio_put(LED_G, !gpio_get(LED_G)); 
                switch (a){
                case 0: 
                    retx = 122 ;
                    rety = 58;
                    larg = 3;
                    a++;
                break;
                case 1: 
                    retx = 112 ;
                    rety = 48;
                    larg = 8;
                    a++;
                break;
                case 2: 
                    retx = 126 ;
                    rety = 62;
                    larg = 1;
                    a=0;
                break;
                }
            }
        }

    }

ssd1306_t oled;
const uint16_t WRAP_PERIOD = 65.535; //valor máximo do contador - WRAP
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

    gpio_init(botA);// declaração dos botões
    gpio_set_dir(botA,GPIO_IN);
    gpio_pull_up(botA);

    gpio_init(botJ);
    gpio_set_dir(botJ,GPIO_IN);
    gpio_pull_up(botJ);

    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);

    gpio_set_irq_enabled_with_callback(botA,GPIO_IRQ_EDGE_FALL,true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botJ,GPIO_IRQ_EDGE_FALL,true, &gpio_irq_handler);

    while (true) {

        
        adc_select_input(1);  
        vrx_value = adc_read();
        if(hab){ 
        if (abs((int)vrx_value - 2048) > margem){
        pwm_set_gpio_level(LED_R, abs((int)vrx_value - 2048)); 
        }else
        pwm_set_gpio_level(LED_R, 0); 
        }
        adc_select_input(0);  
        vry_value = adc_read();
        if(hab){ 
        if (abs((int)vry_value - 2048) > margem){
        pwm_set_gpio_level(LED_B, abs((int)vry_value - 2048)); 
        }else
        pwm_set_gpio_level(LED_B, 0);
        }
        
        
     if (vrx_value/32 < 121 && vrx_value/32 > 7){
       box_x = (uint16_t)vrx_value/32 ;}
    if (vry_value/64 <61  && vry_value/64 > 7){
        box_y = 64 -(uint16_t)vry_value/64;}

        // Atualiza display
        ssd1306_fill(&oled, !cor);
        ssd1306_rect(&oled, box_y, box_x, 8, 8,cor,cor);
        ssd1306_rect(&oled, larg, larg, retx, rety, cor, !cor); // Desenha um retângulo
        ssd1306_send_data(&oled);

        printf("vrx_value: %d, vry_value: %d\n", vrx_value, vry_value);

        sleep_ms(100);  
    }

    return 0;  
}