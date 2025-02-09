<h1 align="center">Tarefa Interfaces de Comunicação Serial - Embarcatech </h1>

<h2>Descrição</h2>
<p>Este projeto utiliza o Raspberry Pi Pico W para exibir caracteres no display SSD1306 (128x64) e controlar LEDs WS2812, além de interagir com botões para realizar ações como alternar o estado de LEDs RGB. O projeto envolve o envio de caracteres digitados no Serial Monitor do Visual Studio Code, bem como o controle de LEDs e exibição de mensagens informativas sobre o estado dos LEDs.</p>

<h2>Funcionalidades do Projeto</h2>

<ul>
    <li><strong>Modificando a Biblioteca <code>font.h</code></strong><br>
        Foram adicionados caracteres minúsculos à biblioteca <code>font.h</code>.</li>
    <li><strong>Entrada de Caracteres via PC</strong><br>
        Foi utilizado o Serial Monitor do VS Code para digitar caracteres. Cada caractere digitado no Serial Monitor é exibido no display SSD1306.<br>
        Quando um número entre 0 e 9 foi digitado, o número é exibido no display e também é mostrado na matriz de LEDs WS2812.</li>
    <li><strong>Interagindo com o Botão A</strong><br>
        Ao pressionar o botão A, o estado do LED RGB Verde é alternado (ligado/desligado). A operação é registrada de duas formas:
        <ul>
            <li>Uma mensagem sobre o estado do LED é exibida no display SSD1306.</li>
            <li>Um texto descritivo sobre a operação é enviado ao Serial Monitor.</li>
        </ul>
    </li>
    <li><strong>Interagindo com o Botão B</strong><br>
        Ao pressionar o botão B, o estado do LED RGB Azul é alternado (ligado/desligado). A operação é registrada de duas formas:
        <ul>
            <li>Uma mensagem sobre o estado do LED é exibida no display SSD1306.</li>
            <li>Um texto descritivo sobre a operação é enviado ao Serial Monitor.</li>
        </ul>
    </li>
</ul>

<h2>Requisitos do Projeto</h2>
<ul>
    <li><strong>Usando Interrupções</strong><br>
        Todas as funcionalidades relacionadas aos botões foram implementadas utilizando rotinas de interrupção (IRQ).</li>
    <li><strong>Implementando Debouncing</strong><br>
        Foi implementado o tratamento do bouncing dos botões via software para garantir que o sinal seja processado corretamente.</li>
    <li><strong>Controlando LEDs</strong><br>
        O projeto inclui o uso do LED RGB e a Matiz de LEDs WS2812.</li>
    <li><strong>Utilizando o Display 128x64</strong><br>
        A utilização de fontes maiúsculas e minúsculas permitem a comunicação via I2C.</li>
    <li><strong>Enviando Informação pela UART</strong><br>
        O projeto utiliza comunicação serial (UART) para enviar mensagens ao Serial Monitor, mostrando o controle e a interação com os LEDs.</li>
</ul>

<h2>Componentes Necessários</h2>
<ul>
    <li>1 x Raspberry Pi Pico W</li>
    <li>1 x Display OLED SSD1306 (128x64)</li>
    <li>1 x LED RGB (com 3 pinos)</li>
    <li>3 x Resistores de 330Ω para limitar a corrente dos LEDs</li>
    <li>1 x Matriz de LEDs WS2812 (5x5)</li>
    <li>2 x Botões para alternar os estados dos LEDs</li>
    <li>1 x Placa BitDogLab</li>
</ul>
<h2>Tecnologias Utilizadas</h2>
<h2>Vídeo</h2>
<p>Link do Vídeo: https://youtu.be/-nepLV9VKVg</p>
<h2>Tecnologias Utilizadas</h2>
<ul>
    <li>Linguagem C</li>
    <li>IDE Visual Studio Code</li>
    <li>Interfaces de Comunicação Serial</li>
     <li>Pico SDK</li>
</ul>

<h2>Clone o Repositório</h2>
<ol>
    <li>Abra o <strong>Prompt de Comando</strong> ou o terminal de sua preferência.</li>
    <li>Clone o repositório usando o Git:
        <pre><code>git clone https://github.com/seu-usuario/seu-repositorio.git</code></pre>
    </li>
    <li>Entre no diretório do projeto:
        <pre><code>cd seu-repositorio</code></pre>
    </li>
</ol>

<h2>Como Compilar e Executar</h2>
<pre>
mkdir build
cd build
cmake ..
make
</pre>


