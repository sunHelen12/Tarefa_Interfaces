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
//matriz de LEDs
#define NUM_PIXELS 25 //número de LEDs
#define OUT_PIN 7 //pino de saída 

//LEDs
#define LED_GREEN 11
#define LED_BLUE 12

//botões
#define BUTTON_0 5
#define BUTTON_1 6

//prototipo da função de interrupção
static void gpio_irq_handler(uint gpio, uint32_t events);

//variáveis globais 
PIO pio;
uint sm;

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



