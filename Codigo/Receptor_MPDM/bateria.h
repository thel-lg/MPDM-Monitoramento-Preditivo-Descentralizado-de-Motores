#ifndef BATERIA_H
#define BATERIA_H

#include <Arduino.h>
#include <Adafruit_ST7735.h> 

extern const int BAT_PIN;
extern const int TFT_LED;
extern Adafruit_ST7735 tft; 

// --- SEUS RESISTORES (NÃO MEXA) ---
const float R1 = 10000.0; // 10k
const float R2 = 27000.0; // 27k
const float FATOR_DIVISAO = (R1 + R2) / R2; 

// Protótipos
void ligarBacklight(int brilho);
void desligarBacklight();
float lerTensaoBateria();
int calcularPorcentagemReal(float tensao);

// --- IMPLEMENTAÇÃO ---

void ligarBacklight(int brilho) {
    #if ESP_IDF_VERSION_MAJOR >= 5
        ledcWrite(TFT_LED, brilho);
    #else
        ledcWrite(0, brilho);
    #endif
}

void desligarBacklight() {
    ligarBacklight(0);
}

float lerTensaoBateria() {
    long soma = 0;
    // Média de 20 leituras para estabilidade
    for(int i=0; i<20; i++) {
        soma += analogRead(BAT_PIN);
        delay(1);
    }
    int valorADC = soma / 20;

    // Calibragem Fina: O ESP32 geralmente lê um pouco baixo. 
    // Se sua bateria estiver dando valor errado, ajuste este 3.3 para 3.25 ou 3.35
    float tensaoNoPino = (valorADC / 4095.0) * 3.33; 
    
    return tensaoNoPino * FATOR_DIVISAO; 
}

// --- NOVA FUNÇÃO: CURVA REAL DE LITHIUM ---
int calcularPorcentagemReal(float volts) {
    // Tabela aproximada para Li-Ion/Li-Po 3.7V sob carga leve
    if (volts >= 4.20) return 100;
    if (volts >= 4.15) return 95;
    if (volts >= 4.10) return 90;
    if (volts >= 4.05) return 85;
    if (volts >= 4.00) return 80;
    if (volts >= 3.90) return 70;
    if (volts >= 3.80) return 60;
    if (volts >= 3.70) return 50; // Tensão nominal
    if (volts >= 3.65) return 40;
    if (volts >= 3.60) return 30;
    if (volts >= 3.55) return 20;
    if (volts >= 3.50) return 10;
    if (volts >= 3.40) return 5;
    return 0; // Bateria morta
}

#endif