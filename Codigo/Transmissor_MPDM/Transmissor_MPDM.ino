// ======================================================================
// TRANSMISSOR FINAL 
// ======================================================================

#include <LoRa.h>
#include <WiFi.h>
#include "esp_bt.h"
#include <Wire.h> 

// Estrutura 
struct DadosSensores {
    float corrente;
    float temperatura;
    float ax;
    float ay;
    float az;
};

enum StatusType { STATUS_OK, STATUS_ERRO_IMU, STATUS_ERRO_TEMP, STATUS_ERRO_CORRENTE, STATUS_ERRO_LORA };

// Includes
#include "sensor_bmi160.h"
#include "sensor_ds18b20.h"
#include "sensor_sct013.h"

#define BOTAO_INICIAR_PIN 32 
#define LED_R_PIN 25
#define LED_G_PIN 26
#define LED_B_PIN 33

#define LORA_SS 5
#define LORA_RST 17
#define LORA_DIO0 16

enum SystemState { STANDBY, TRANSMITTING };
SystemState systemState = STANDBY;

DadosSensores dadosAtuais;
bool lora_ok = false;

// Status LED
StatusType errosAtivos[5];
int totalDeErros = 0;
int indiceDoErroAtual = 0;
unsigned long ultimoTempoPiscada = 0; 
bool ledPiscandoApagado = false; 

const long INTERVALO_ENVIO_LORA = 1000;
unsigned long timerLoRa = 0;

void setLedColor(bool red, bool green, bool blue) {
    digitalWrite(LED_R_PIN, red ? LOW : HIGH);
    digitalWrite(LED_G_PIN, green ? LOW : HIGH);
    digitalWrite(LED_B_PIN, blue ? LOW : HIGH);
}

void setup_led_status() {
    pinMode(LED_R_PIN, OUTPUT); pinMode(LED_G_PIN, OUTPUT); pinMode(LED_B_PIN, OUTPUT);
    setLedColor(false, false, false); 
}

void gerenciarLedDeStatus(const DadosSensores &dados) {
    // 1. AZUL (Prioridade Total)
    if (systemState == TRANSMITTING) {
        setLedColor(false, false, true); return; 
    }
    // 2. ERRO
    if (totalDeErros > 0) {
        if (millis() - ultimoTempoPiscada > 800) {
            ultimoTempoPiscada = millis();
            if (ledPiscandoApagado) {
                setLedColor(false, false, false); 
                indiceDoErroAtual = (indiceDoErroAtual + 1) % totalDeErros;
                ledPiscandoApagado = false;
            } else {
                StatusType erro = errosAtivos[indiceDoErroAtual];
                if(erro == STATUS_ERRO_CORRENTE) setLedColor(true, true, true); // Branco
                else if(erro == STATUS_ERRO_TEMP) setLedColor(false, true, true); // Ciano
                else setLedColor(true, false, false); // Vermelho
                ledPiscandoApagado = true;
            }
        }
        return; 
    }
    // 3. OK / CARGA
    if (dados.corrente > 0.15) {
        if (millis() - ultimoTempoPiscada > 100) {
            ultimoTempoPiscada = millis();
            setLedColor(false, !digitalRead(LED_G_PIN), false); // Pisca Verde
        }
    } else {
        setLedColor(false, true, false); // Verde Fixo
    }
}

void setup_lora() {
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    if (!LoRa.begin(433E6)) {
        lora_ok = false; return;
    }
    // --- CONFIGURAÇÃO DE ALCANCE  ---
    LoRa.setTxPower(20);
    LoRa.setSyncWord(0x12);
    LoRa.setSpreadingFactor(9); // SF9 (Balanço alcance/velocidade)
    LoRa.setSignalBandwidth(125E3); // 125kHz (Mais estável que 62.5k)
    LoRa.setCodingRate4(8);
    lora_ok = true;
}

void setup() {
    Serial.begin(115200);
    Wire.begin(); WiFi.mode(WIFI_OFF); btStop();
    pinMode(BOTAO_INICIAR_PIN, INPUT_PULLUP); 
    setup_led_status();
    setup_bmi160(); setup_ds18b20(); setup_sct013(); 
    setup_lora(); 
}

void loop() {
    // Leitura
    bool imu_ok = loop_bmi160(dadosAtuais);
    bool temp_ok = loop_ds18b20(dadosAtuais);
    bool current_ok = loop_sct013(dadosAtuais);
    
    totalDeErros = 0;
    if (!lora_ok) errosAtivos[totalDeErros++] = STATUS_ERRO_LORA;
    if (!imu_ok) errosAtivos[totalDeErros++] = STATUS_ERRO_IMU;
    if (!temp_ok) errosAtivos[totalDeErros++] = STATUS_ERRO_TEMP;
    if (!current_ok) errosAtivos[totalDeErros++] = STATUS_ERRO_CORRENTE;

    // Botão
    static int lastBtn = HIGH;
    static unsigned long dbBtn = 0;
    int leituraBtn = digitalRead(BOTAO_INICIAR_PIN); 
    if (leituraBtn == LOW && lastBtn == HIGH) {
        if ((millis() - dbBtn) > 300) {
            dbBtn = millis();
            systemState = (systemState == STANDBY) ? TRANSMITTING : STANDBY;
        }
    }
    lastBtn = leituraBtn;

    // Transmissão
    if (systemState == TRANSMITTING) {
        if (millis() - timerLoRa > INTERVALO_ENVIO_LORA) {
            timerLoRa = millis();
            if(lora_ok) {
                LoRa.beginPacket();
                LoRa.write((uint8_t*)&dadosAtuais, sizeof(DadosSensores));
                LoRa.endPacket();
                Serial.println("TX OK");
            } else {
                setup_lora(); // Tenta reviver
            }
        }
    }
    gerenciarLedDeStatus(dadosAtuais);
}