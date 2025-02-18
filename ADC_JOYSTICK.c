#include <stdio.h>             
#include "pico/stdlib.h"       
#include "hardware/adc.h"      
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "Funcoes/ssd1306.c"

#define VRX_PIN 27   
#define VRY_PIN 26   
#define SW_PIN 22    
#define LED_VERDE 11  
#define LED_AZUL 12  
#define LED_VERMELHO 13  
#define botão_A 5
#define botão_JOY 22
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define dead_angle 75

ssd1306_t ssd;
bool cor = true;
static volatile uint32_t last_time = 0;
static volatile bool estado_LED = true, estado_BORDA = false;
static float duty_cycle_VERMELHO,duty_cycle_AZUL;
static volatile uint pos_quadrado[2]={61,29}; 
void gpio_irq_handler(uint gpio, uint32_t events);
void setup_pwm(uint gpio, float duty_cycle);
int main() {
  stdio_init_all();
  adc_init();
  adc_gpio_init(VRX_PIN); 
  adc_gpio_init(VRY_PIN); 
  
  
  //Inciailizando e configurando os pinos dos botões e LEDs
  gpio_init(botão_A);//Inicializa o pino do botão A
  gpio_set_dir(botão_A, GPIO_IN);// Configura o pino como entrada
  gpio_pull_up(botão_A);// Habilita o pull-up interno
  gpio_init(botão_JOY);//Inicializa o pino do botão A
  gpio_set_dir(botão_JOY, GPIO_IN);// Configura o pino como entrada
  gpio_pull_up(botão_JOY);// Habilita o pull-up interno 
  gpio_init(LED_VERDE);
  gpio_set_dir(LED_VERDE, GPIO_OUT);
  gpio_put(LED_VERDE, false); 
  gpio_init(LED_AZUL);
  gpio_set_dir(LED_AZUL, GPIO_OUT);
  gpio_put(LED_AZUL, false); 
  gpio_init(LED_VERMELHO);
  gpio_set_dir(LED_VERMELHO, GPIO_OUT);
  gpio_put(LED_VERMELHO, false);

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA); // Pull up the data line
  gpio_pull_up(I2C_SCL); // Pull up the clock line
  //ssd1306_t ssd; // Inicializa a estrutura do display
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd); // Configura o display
  ssd1306_send_data(&ssd); // Envia os dados para o display
  gpio_set_irq_enabled_with_callback(botão_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  gpio_set_irq_enabled(botão_JOY, GPIO_IRQ_EDGE_FALL,true);
  while (true) {
    adc_select_input(1); 
    uint16_t vrx_value = adc_read(); 
    adc_select_input(0); 
    uint16_t vry_value = adc_read();  
    if((vrx_value-2047)>dead_angle) {
        duty_cycle_VERMELHO = (vrx_value-2047)/2047.0;
        pos_quadrado[0] = 61+duty_cycle_VERMELHO*58;
    }else if((2047-vrx_value)>dead_angle){
        duty_cycle_VERMELHO = (2047-vrx_value)/2047.0;
        pos_quadrado[0] = 61-duty_cycle_VERMELHO*58;
    }else{
        pos_quadrado[0] = 61;
        duty_cycle_VERMELHO = 0;
    }
    if ((vry_value-2047)>dead_angle) {
        duty_cycle_AZUL = (vry_value-2047)/2047.0;
        pos_quadrado[1] = 29-duty_cycle_AZUL*27;
    }else if((2047-vry_value)>dead_angle) {
        duty_cycle_AZUL = (2047-vry_value)/2047.0;
        pos_quadrado[1] = 29+duty_cycle_AZUL*27;
    }else{
        pos_quadrado[1] = 29;
        duty_cycle_AZUL = 0;
    }
    if(estado_LED){
        setup_pwm(LED_VERMELHO, duty_cycle_VERMELHO);
        setup_pwm(LED_AZUL, duty_cycle_AZUL);
    }else{
        setup_pwm(LED_VERMELHO, 0);
        setup_pwm(LED_AZUL, 0); 
    }
    ssd1306_fill(&ssd, !cor); // Limpa o display
    if(estado_BORDA){
        ssd1306_rect(&ssd, 0, 0, 128, 64, cor, !cor); // Desenha borda
    }
    ssd1306_draw_string(&ssd,"0",pos_quadrado[0],pos_quadrado[1]);// Desenha o quadrado
    ssd1306_send_data(&ssd); 
    printf("VRX: %u, VRY: %u, DCV: %.2f, DCA: %.2f\n", vrx_value, vry_value,duty_cycle_VERMELHO,duty_cycle_AZUL);//Apenas para debug
    sleep_ms(100);
  }
  return 0;
}

void setup_pwm(uint gpio, float duty_cycle) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);// Configura GPIO para PWM
    uint slice = pwm_gpio_to_slice_num(gpio);// Obtém o slice associado ao pino GPIO
    pwm_set_clkdiv(slice, 250.0f);// Configura divisor de clock para 250 (clock base de 500kHz)
    pwm_set_wrap(slice, 10000);// Define o contador máximo (wrap) para 10.000
    pwm_set_gpio_level(gpio, (uint16_t)(10000 * (duty_cycle / 100.0)));// Define o nível do duty cycle (0,12% de 10.000 -> 12)
    pwm_set_enabled(slice, true);// Ativa o PWM
}

void gpio_irq_handler(uint gpio, uint32_t events){
  uint32_t current_time = to_us_since_boot(get_absolute_time());// Obtém o tempo atual em microssegundos
  if (current_time - last_time > 200000){ // Verifica se passou tempo suficiente desde o último evento com 200 ms de debouncing 
      last_time = current_time; // Atualiza o tempo do último evento
      if(gpio == botão_A){
        estado_LED = !estado_LED;//Altera a varivel que ativa/desativa o estado dos LEDs
        }
      else if(gpio == botão_JOY){
        estado_BORDA = !estado_BORDA;
        setup_pwm(LED_VERDE, estado_BORDA);
      }
  }
}