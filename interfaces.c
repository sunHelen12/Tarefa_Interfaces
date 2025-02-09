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
#define DISPLAY_ADDR 0x3C

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
// estrutura do display
ssd1306_t ssd;

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


// Função para desenhar strings no display
void draw_text(ssd1306_t *display, int x, int y, const char *text) {
    while (*text) {
        ssd1306_draw_char(display, *text, x, y);
        x += 6; // Avança para a próxima posição (ajuste conforme necessário)
        text++;
    }
}
// Rotina para atualizar o display com o estado dos LEDs
void atualizar_display() {
    ssd1306_fill(&ssd, false); // Limpa o display

    char msg_verde[10];
    char msg_azul[10];

    if (gpio_get(LED_GREEN)) {
        sprintf(msg_verde, "Verde ON");
    } else {
        sprintf(msg_verde, "Verde OFF");
    }

    if (gpio_get(LED_BLUE)) {
        sprintf(msg_azul, "Azul ON");
    } else {
        sprintf(msg_azul, "Azul OFF");
    }

    draw_text(&ssd, 10, 10, msg_verde);
    draw_text(&ssd, 10, 30, msg_azul);

    ssd1306_send_data(&ssd); // Atualiza o display
}


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

// vetores de animação dos números
double *numeros[10] = {numero_zero, numero_um, numero_dois, numero_tres, numero_quatro, 
                       numero_cinco, numero_seis, numero_sete, numero_oito, numero_nove};

// exibir número na matriz WS2812
void mostrar_numero(int numero) {
    if (numero >= 0 && numero <= 9) {
        desenho_pio(numeros[numero]);
    }
}

//rotina principal
int main(){
    pio = pio0; // para carregarmos nossa PIO

    //inicializa a biblioteca padrão
    stdio_init_all();

     // Configura LEDs
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_put(LED_GREEN, 0);

    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_put(LED_BLUE, 0);

    // Configura botões
    gpio_init(BUTTON_0);
    gpio_set_dir(BUTTON_0, GPIO_IN);
    gpio_pull_up(BUTTON_0);

    gpio_init(BUTTON_1);
    gpio_set_dir(BUTTON_1, GPIO_IN);
    gpio_pull_up(BUTTON_1);     

    // Configura SSD1306
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

     // Configura PIO para matriz de LEDs WS2812
    pio = pio0;
    uint offset = pio_add_program(pio, &interfaces_program);
    sm = pio_claim_unused_sm(pio, true);
    interfaces_program_init(pio, sm, offset, OUT_PIN);    

    // Configura interrupção nos botões
    gpio_set_irq_enabled_with_callback(BUTTON_0, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_1, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);    

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, DISPLAY_ADDR, I2C_PORT);
    ssd1306_config(&ssd);    
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    bool cor = true;    

    printf("Sistema inicializado! Digite caracteres no Serial Monitor.\n");

    while (true) {
        sleep_ms(1000);
        if (stdio_usb_connected()){
            char c;
            if (scanf("%c", &c) == 1){
                printf("Recebido: '%c'\n", c);
                 // Verifica se o caractere é um número de 0 a 9
                if (c >= '0' && c <= '9') {
                    int numero = c - '0'; // Converte o caractere para o número inteiro
                    mostrar_numero(numero); // Exibe o número na matriz de LEDs
                }
                
                cor = !cor;
                ssd1306_fill(&ssd, !cor);
                ssd1306_rect(&ssd, 3, 3, 112, 58, cor, !cor);
                
                ssd1306_draw_char(&ssd, c, 20, 30);

                ssd1306_send_data(&ssd);                
            }
            sleep_ms(40);
            
        }
    }
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
                printf("LED Verde: %s\n", gpio_get(LED_GREEN) ? "Ligado" : "Desligado");
            }           
        }

        if (gpio == BUTTON_1) { // botão 1 pressionado
            if (gpio_get(LED_GREEN)) { 
                gpio_put(LED_GREEN, 0); // apaga o LED verde se estiver aceso
                printf("LED Verde: Desligado\n");
            } else {
                gpio_xor_mask(1 << LED_BLUE); // inverte o estado do LED azul
                printf("LED Azul: %s\n", gpio_get(LED_BLUE) ? "Ligado" : "Desligado");                
            }      
        }
        // Atualiza o display com o novo estado dos LEDs
        atualizar_display();
      
        last_time = current_time; // atualiza o tempo do último evento
    }
}


