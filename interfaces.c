#include <stdio.h>
#include <stdlib.h>
#include <pico/stdlib.h>
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "interfaces.pio.h"

//display
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

//configuração do UART
#define UART_ID uart0 // seleciona a UART0
#define BAUD_RATE 115200 // define a taxa de transmissão
#define UART_TX_PIN 0 // pino GPIO usado para TX
#define UART_RX_PIN 1 // pino GPIO usado para RX

//configuração da matriz de LEDs
#define NUM_PIXELS 25 //número de LEDs
#define OUT_PIN 7 //pino de saída 

//configuração dos LEDs
#define LED_GREEN 11
#define LED_BLUE 12

// configurações dos botões
#define BUTTON_0 5
#define BUTTON_1 6

//prototipo da função de interrupção
static void gpio_irq_handler(uint gpio, uint32_t events);

//variáveis globais 
PIO pio;
uint sm;

//variáveis voláteis
static volatile uint32_t last_time = 0; //armazena o último evento de temo (microssegundos)

//vetores com animação dos números
double numero_zero[25] =    {0.0, 0.0, 0.3, 0.3, 0.3,
                             0.3, 0.0, 0.3, 0.0, 0.0, 
                             0.0, 0.0, 0.3, 0.0, 0.3,
                             0.3, 0.0, 0.3, 0.0, 0.0,
                             0.0, 0.0, 0.3, 0.3, 0.3};

double numero_um[25] =      {0.0, 0.0, 0.0, 0.0, 0.3,
                             0.3, 0.0, 0.0, 0.0, 0.0, 
                             0.0, 0.0, 0.0, 0.0, 0.3,
                             0.3, 0.0, 0.0, 0.0, 0.0,
                             0.0, 0.0, 0.0, 0.0, 0.3};

double numero_dois[25] =    {0.0, 0.0, 0.3, 0.3, 0.3,
                             0.3, 0.0, 0.0, 0.0, 0.0, 
                             0.0, 0.0, 0.5, 0.3, 0.3,
                             0.0, 0.0, 0.5, 0.0, 0.0,
                             0.0, 0.0, 0.3, 0.3, 0.3};  

double numero_tres[25] =    {0.0, 0.0, 0.3, 0.3, 0.3,
                             0.3, 0.0, 0.0, 0.0, 0.0, 
                             0.0, 0.0, 0.3, 0.3, 0.3,
                             0.3, 0.0, 0.0, 0.0, 0.0,
                             0.0, 0.0, 0.3, 0.3, 0.3};  

double numero_quatro[25] =  {0.0, 0.0, 0.3, 0.0, 0.3,
                             0.3, 0.0, 0.3, 0.0, 0.0, 
                             0.0, 0.0, 0.3, 0.3, 0.3,
                             0.3, 0.0, 0.0, 0.0, 0.0,
                             0.0, 0.0, 0.0, 0.0, 0.3}; 

double numero_cinco[25] =  {0.0, 0.0, 0.3, 0.3, 0.3,
                            0.0, 0.0, 0.3, 0.0, 0.0, 
                            0.0, 0.0, 0.3, 0.3, 0.3,
                            0.3, 0.0, 0.0, 0.0, 0.0,
                            0.0, 0.0, 0.3, 0.3, 0.3}; 

double numero_seis[25] =   {0.0, 0.0, 0.3, 0.3, 0.3,
                            0.0, 0.0, 0.3, 0.0, 0.0, 
                            0.0, 0.0, 0.3, 0.3, 0.3,
                            0.3, 0.0, 0.3, 0.0, 0.0,
                            0.0, 0.0, 0.3, 0.3, 0.3};
                             
double numero_sete[25] =   {0.0, 0.0, 0.3, 0.3, 0.3,
                            0.3, 0.0, 0.3, 0.0, 0.0, 
                            0.0, 0.0, 0.0, 0.0, 0.3,
                            0.3, 0.0, 0.0, 0.0, 0.0,
                            0.0, 0.0, 0.0, 0.0, 0.3};

double numero_oito[25] =   {0.0, 0.0, 0.3, 0.3, 0.3,
                            0.3, 0.0, 0.3, 0.0, 0.0, 
                            0.0, 0.0, 0.3, 0.3, 0.3,
                            0.3, 0.0, 0.3, 0.0, 0.0,
                            0.0, 0.0, 0.3, 0.3, 0.3};

double numero_nove[25] =   {0.0, 0.0, 0.3, 0.3, 0.3,
                            0.3, 0.0, 0.3, 0.0, 0.0, 
                            0.0, 0.0, 0.3, 0.3, 0.3,
                            0.3, 0.0, 0.0, 0.0, 0.0,
                            0.0, 0.0, 0.3, 0.3, 0.3};

//rotina pra definição de cores do led
uint32_t matrix_rgb(double r, double g, double b){
    unsigned char R, G, B;
    R = r * 255;
    G = g * 255;
    B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}  

//rotina para acionar a matriz de LEDs - ws2812b
//adiciona a cor aos LEDs
void desenho_pio(double *desenho){
    uint32_t valor_led;
    for (int16_t i = 0; i < NUM_PIXELS; i++){
        valor_led = matrix_rgb(0.0, desenho[24 - i], desenho[24 - i]); //adiciona a cor ciano aos LEDs da Matriz
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

//rotina principal
int main(){
    pio = pio0; // para carregarmos nossa PIO
    
    //inicializa a biblioteca padrão
    stdio_init_all();

    // configura os LEDs
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    
    // inicializa os botões com pull-up interno
    gpio_init(BUTTON_0);
    gpio_set_dir(BUTTON_0, GPIO_IN);
    gpio_pull_up(BUTTON_0);

    gpio_init(BUTTON_1);
    gpio_set_dir(BUTTON_1, GPIO_IN);
    gpio_pull_up(BUTTON_1);

    //configura os pinos GPIO para a UART
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART); //configura o pino 0 para TX
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART); //configura o pino 1 para RX

    //configurações da PIO
    uint offset = pio_add_program(pio, &interfaces_program);
    sm = pio_claim_unused_sm(pio, true);
    interfaces_program_init(pio, sm, offset, OUT_PIN);

    //configuração de interrupção com callback para os botões
    gpio_set_irq_enabled_with_callback(BUTTON_0, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);   
    gpio_set_irq_enabled_with_callback(BUTTON_1, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    

}

// rotina de interrupção
void gpio_irq_handler(uint gpio, uint32_t events) {
    // armazena o tempo atual em microssegundos
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // verifica se passou tempo suficiente desde o último evento (debouncing)
    if (current_time - last_time > 200000) { // 200ms de debounce
        if (gpio == BUTTON_0) { // botão 0 pressionado
            if (gpio_get(LED_BLUE)) { 
                gpio_put(LED_BLUE, 0); // apaga o LED azul se estiver aceso
                printf("LED Azul: Desligado\n");
            } else {
                gpio_xor_mask(1 << LED_GREEN); // inverte o estado do LED verde
                printf("LED Verde: %s\n", gpio_get(LED_GREEN) ? "Desligado" : "Ligado");
                exibir_no_display(gpio_get(LED_GREEN) ? "LED Verde ON" : "LED Verde OFF");
            }           
        }

        if (gpio == BUTTON_1) { // botão 1 pressionado
            if (gpio_get(LED_GREEN)) { 
                gpio_put(LED_GREEN, 0); // apaga o LED verde se estiver aceso
                printf("LED Verde: Desligado\n");
            } else {
                gpio_xor_mask(1 << LED_BLUE); // inverte o estado do LED azul
                printf("LED Azul: %s\n", gpio_get(LED_BLUE) ? "Desligado" : "Ligado");
                exibir_no_display(gpio_get(LED_BLUE) ? "LED Azul ON" : "LED Azul OFF");
            }      
            
        }

        last_time = current_time; // atualiza o tempo do último evento
    }
}

