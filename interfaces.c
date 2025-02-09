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
#define UART_ID uart0 // Seleciona a UART0
#define BAUD_RATE 115200 // Define a taxa de transmissão
#define UART_TX_PIN 0 // Pino GPIO usado para TX
#define UART_RX_PIN 1 // Pino GPIO usado para RX

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
static volatile uint a = 1; //vai incrementar as vezes que o botão será apertado
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

// guardando todos os números em um vetor
double *numeros [10] = {
    numero_zero, numero_um, numero_dois, numero_tres, numero_quatro,
    numero_cinco, numero_seis, numero_sete, numero_oito, numero_nove
};

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

    //inicializa a biblioteca padrão
    stdio_init_all();


    //configuração de interrupção com callback para os botões
    gpio_set_irq_enabled_with_callback(BUTTON_0, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);   
    gpio_set_irq_enabled_with_callback(BUTTON_1, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

}

//rotina de interrupção 
void gpio_irq_handler(uint gpio, uint32_t events){
    //armazena o tempo atual em microssegundos
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    //será verificado se o tempo que passou foi suficiente desde o último evento 
    if (current_time - last_time > 200000){ //200ms de deboucing
        if (gpio == BUTTON_0){
           
        }

        if (gpio == BUTTON_1){
           
        }
        last_time = current_time; //atualizar o tempo
    }
    //atualiza a matriz com o novo número e implementa a cor dos LEDs (roxa)    
}