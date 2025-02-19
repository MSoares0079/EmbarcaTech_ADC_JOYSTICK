# Conversor analógico digital(ADC) utilizando um joystick
Este projeto tem como objetivo mostrar as funcionalidades do ADC do RP2040 utilizando um joystick como entrada na placa da BitDogLab, além da representação visual através de um display.

## Como baixar e executar o código
    1-Baixar ou utilizar git clone no repositório.
          git clone https://github.com/MSoares0079/EmbarcaTech_ADC_JOYSTICK.git
    2-Ter o VS Code instalado e devidamente configurado.
    3-Ter a extensão do Raspberry Pi Pico instalada no VS Code.
    4-Compilar o arquivo "ADC_JOYSTICK.c" utilizando o compilador do Raspberry Pi Pico.
    5-O código pode ser tanto simulado pelo "diagram.json" apartir da extensão Wokwi quanto compilado na placa da BitDogLab.

## Estrutura do código

A função principal (int main) inicializa e configura os pinos que serão utilizados no código sendo eles: botões,LEDs,joystick e o display, além de ativar interrupção para os botões.

No loop infinito ocorre a multiplexação dos canais do ADC, em que é feita a conversão primeiro do eixo x e depois do eixo y, salvando os valores em variaveis.
Após isso, ocorre a comparação destes valores com constantes, idetenficando o quanto o joystick se afastou da posição central(0 a 1).
Então é deslocado um quadrado, inicialmente centralizado no display, proporcionalmente a essas diferenças.
Além disso, caso a variavél (estado_LED) seja verdadeiro,
os LEDs azul e vermelho tem um PWM ligados com duty cycle iguais a diferença entre,respectivamente, posição x e y e o ponto central.

Ao precionar o botão A da placa da BitDogLab a variavel (estado_LED) é alterado, ou seja desativando os LEDs controlados pelo Joystick. 
E ao precionar o botão do Joystick é alterado a variavel (estado_BORDA) em que passa a ser desenhado uma borda no display, além de ativar o PWM do LED verde.



## Link do vídeo demonstrando o código funcionando na placa
https://drive.google.com/file/d/15CkIwpoa7k7o8Y6W6DI-A0IKzV-qgGxt/view?usp=sharing
