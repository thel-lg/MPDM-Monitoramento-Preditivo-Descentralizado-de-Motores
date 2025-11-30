#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <RTClib.h>
#include <LoRa.h>
#include <SD.h>
#include <WiFi.h>
#include "esp_wifi.h"

// ESTRUTURA
struct DadosSensores {
    float corrente; float temperatura; float ax; float ay; float az;
};
DadosSensores dadosRecebidos;

// PINOS
#define LORA_SS    5
#define LORA_RST   17
#define LORA_DIO0  16
#define LORA_SCK   18
#define LORA_MOSI  23
#define LORA_MISO  19 

#define TFT_CS     15
#define TFT_DC     4
#define TFT_RST    25
const int TFT_LED    = 26;

const int BUZZER_PIN = 14;
const int BAT_PIN    = 34;
const int SD_CS_PIN  = 13; 

// BOTÕES
const int BOTAO_SET  = 27; 
const int BOTAO_PWR  = 33; 
const int BOTAO_DIR  = 32; // Digital
const int BOTAO_ESQ  = 39; // Analógico SN

// GLOBAIS
const long INTERVALO_GRAVACAO_SD = 15000;
const long INTERVALO_PACOTE_OK = 3000;      
const long INTERVALO_PACOTE_ATRASO = 10000; 
const int NUM_PONTOS_GRAFICO = 100;
const float ALPHA = 0.8; 
const bool DEBUG_LORA = true;
const long TIMEOUT_LORA = 300000; 

int falhaLoRaCount = 0;
int falhaRTCCount = 0;
int falhaSDCount = 0;
unsigned long lastSDTryMs = 0;
bool falhaHardwareLoRa = false;
const int MAX_RECOVERY_ATTEMPTS = 5; 

// Controle SD Seguro
bool sdCardOperacional = false;
bool falhaSD = false;
String erroSD = "OK";

bool sistemaLigado = true;
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
RTC_DS3231 rtc;

String ultimoPacoteLoRaBruto = "Aguardando..."; 

float corrente = 0, temperatura = 0, vibracaoRMS = 0;
float eixoX = 0, eixoY = 0, eixoZ = 0;
float rssiLoRa = 0;
float padraoCorrente = 0, padraoVibracao = 0, padraoTemperatura = 0;
bool padroesDefinidos = false;
bool primeiraLeituraAposReset = true; 

// Navegação
int telaAtual = 1;
int appSelecionado = 1; 
int menuAjustesSelecionado = 0;
int menuPadroesSelecionado = 0;
int dadosSubTela = 0; 
int devMenuSelecionado = 0;
int devSubPagina = 0;
int devSDAcaoSelecionada = 0;
int scrollIndex = 0;

// Estado
bool emProcessoDeCalibracao = false;
unsigned long inicioCalibracaoMs = 0;
float somaCalibracaoI = 0, somaCalibracaoV = 0, somaCalibracaoT = 0;
int contadorCalibracao = 0;

bool falhaModuloVisivel = false;
bool falhaModuloAck = false; // IMUNIDADE DO POPUP
bool alertaAmareloAtivo = false; 
bool alertaVermelhoAtivo = false; 
unsigned long ultimoPacoteLoraMs = 0;
unsigned long ultimoTempoGravacaoSD = 0;
unsigned long lastUpdateMs = 0;

bool falhaLoRa = false;
bool falhaRTC = false;
String erroRTC = "OK";
String erroLoRa = "OK";

float historicoRMS[NUM_PONTOS_GRAFICO] = {0};
float historicoX[NUM_PONTOS_GRAFICO] = {0};
float historicoY[NUM_PONTOS_GRAFICO] = {0};
float historicoZ[NUM_PONTOS_GRAFICO] = {0};
int indiceHistorico = 0;
float tempMin = 0, tempMax = 0, corrMin = 0, corrMax = 0;

#include "Relogio.h"
#include "Display.h" 
#include "Calibracao.h"
#include "alerta.h"
#include "Cartao.h"
#include "bateria.h"

// --- FUNÇÕES ---
void bip(int duracao) {
    analogWrite(BUZZER_PIN, 20); delay(duracao); analogWrite(BUZZER_PIN, 0);
}

void destravarSPI() {
    digitalWrite(SD_CS_PIN, HIGH); delay(1);
}

bool lerBotaoDigital(int pino) {
  if (digitalRead(pino) == LOW) { delay(50); if (digitalRead(pino) == LOW) return true; }
  return false;
}

bool lerBotaoEsquerdaAnalogico() {
    long soma = 0;
    for(int i=0; i<3; i++) { soma += analogRead(BOTAO_ESQ); delay(2); }
    int leitura = soma / 3;
    if (leitura < 2500) return true; // Calibrado para 5k no SN
    return false;
}

bool iniciarLoRa(bool reset = false) {
    destravarSPI();
    if(reset) {
        LoRa.end();
        pinMode(LORA_RST, OUTPUT);
        digitalWrite(LORA_RST, LOW); delay(20);
        digitalWrite(LORA_RST, HIGH); delay(50);
    }
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    if (!LoRa.begin(433E6)) {
        falhaLoRa = true; erroLoRa = "Nao encontrado"; 
        if(reset) falhaHardwareLoRa = true;
        return false;
    }
    LoRa.setSyncWord(0x12); LoRa.setSpreadingFactor(9); LoRa.setSignalBandwidth(125E3); LoRa.setCodingRate4(8);
    falhaLoRa = false; falhaHardwareLoRa = false; erroLoRa = "OK"; 
    return true;
}

void verificarSaudeModulos() {
  // 1. RTC
  if (!falhaRTC) { 
      bool rtcOk = rtc.now().year() >= 2024; 
      if (!rtcOk) { falhaRTC = true; erroRTC = "Data"; }
  }

  // 2. LoRa Watchdog
  unsigned long tempoDesdeUltimoPacote = millis() - ultimoPacoteLoraMs;
  
  if (tempoDesdeUltimoPacote <= INTERVALO_PACOTE_OK) {
      erroLoRa = "OK"; falhaLoRa = false;
  } 
  else if (tempoDesdeUltimoPacote <= INTERVALO_PACOTE_ATRASO) {
      erroLoRa = "Instavel"; falhaLoRa = false; 
  } 
  else {
      erroLoRa = "SEM SINAL"; falhaLoRa = true;
      // Mantém valores congelados conforme pedido
  }
  
  // 3. LÓGICA DO POPUP
  
  if(!falhaRTC && !falhaLoRa && !falhaSD) {
      falhaModuloAck = false; // Reset da imunidade (está saudável)
  }

  // Calcula se deve mostrar o pop-up AGORA
    bool temErroCritico = (falhaLoRa || falhaRTC || falhaSD);
  falhaModuloVisivel = temErroCritico && !falhaModuloAck;
}

void receberLoraBinario();

// SETUP
void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(BUZZER_PIN, OUTPUT); analogWrite(BUZZER_PIN, 0);
  pinMode(BOTAO_SET, INPUT_PULLUP); pinMode(BOTAO_PWR, INPUT_PULLUP);
  pinMode(BOTAO_DIR, INPUT_PULLUP); pinMode(BOTAO_ESQ, INPUT); 
  pinMode(BAT_PIN, INPUT);
  
  pinMode(SD_CS_PIN, OUTPUT); digitalWrite(SD_CS_PIN, HIGH); delay(10);

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  iniciarDisplay();

  telaInicialProfissional("Iniciando LoRa...", 0.25); iniciarLoRa(true);

  telaInicialProfissional("Verificando RTC...", 0.50); Wire.begin();
  if (!rtc.begin()) { falhaRTC = true; erroRTC = "FALHA I2C"; }
  else {
  
      //rtc.adjust(DateTime(2025, 11, 27, 8, 17, 0)); 
      if (rtc.lostPower() || rtc.now().year() < 2025) rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      falhaRTC = false;
  }
  
  telaInicialProfissional("Montando SD...", 0.75);
  if (!iniciarSDCard_Seguro()) { 
      erroSD = "SD AUSENTE"; falhaSD = true;
  }

  WiFi.mode(WIFI_OFF); btStop();

  telaInicialProfissional("Sistema pronto!", 1.0);
  bip(50); delay(1000);
  
  desenharTelaAtual();
  ultimoPacoteLoraMs = millis(); 
}

// LOOP
void loop() {
  delay(10); 

  // Tenta ler LoRa antes de tudo
  destravarSPI();
  receberLoraBinario(); 

  // === LEITURA BOTÕES ===
  static unsigned long ultimoClique = 0;
  bool acaoBotao = false;
  
  if (millis() - ultimoClique > 200) { 
    bool curSet = lerBotaoDigital(BOTAO_SET); 
    bool curDir = lerBotaoDigital(BOTAO_DIR); 
    bool curEsq = false;
    if (lerBotaoEsquerdaAnalogico()) { delay(50); if (lerBotaoEsquerdaAnalogico()) curEsq = true; }

    // --- BOTÃO POWER (Lógica Independente) ---
    if (lerBotaoDigital(BOTAO_PWR)) {
        bip(50);
        sistemaLigado = !sistemaLigado;
        if (sistemaLigado) {
            ligarBacklight(140); telaInicialProfissional("Reiniciando...", 0.5);
            delay(500); telaAtual = 1; desenharTelaAtual(); ultimoPacoteLoraMs = millis();
        } else { telaDesligando(); }
        ultimoClique = millis();
        return; // Sai do loop
    }

    if (!sistemaLigado) { desligarBacklight(); tft.fillScreen(ST77XX_BLACK); return; }

    if (falhaModuloVisivel) {
            if (curSet) {
            bip(30);
            falhaModuloVisivel = false;
            falhaModuloAck = true; // IMUNIDADE ATIVADA!
            // Limpa a tela e volta pro menu
            tft.fillScreen(COR_FUNDO);
            desenharTelaAtual(); 
            ultimoClique = millis();
        }
        else {
             desenharPopupFalha();
        }
        return; 
    }

    // --- NAVEGAÇÃO NORMAL ---
    if (curEsq || curDir || curSet) {
        ultimoClique = millis(); 
        bip(30); 

        if (curSet) {
          if (telaAtual == 1) { 
            if (appSelecionado == 0) telaAtual = 0;
            else if (appSelecionado == 1) { telaAtual = 2; dadosSubTela = 0; }
            else if (appSelecionado == 2) telaAtual = 3;
            desenharTelaAtual();
          } else if (telaAtual == 3) { 
            if (menuAjustesSelecionado == 0) { telaAtual = 6; menuPadroesSelecionado = 0; }
            else if (menuAjustesSelecionado == 1) { telaAtual = 5; }
            else if (menuAjustesSelecionado == 2) { telaAtual = 4; devMenuSelecionado = 0; }
            desenharTelaAtual();
          } else if (telaAtual == 4) { 
            if (devMenuSelecionado == 0) { telaAtual = 7; devSubPagina = 0; }
            else if (devMenuSelecionado == 1) { telaAtual = 8; devSubPagina = 0; }
            else if (devMenuSelecionado == 2) { telaAtual = 9; devSubPagina = 0; devSDAcaoSelecionada = 0; }
            else if (devMenuSelecionado == 3) { toggleModoGrafico(); }
            desenharTelaAtual();
          } else if (telaAtual == 7 && devSubPagina == 1) { mostrarTelaReset("LoRa"); iniciarLoRa(true); ultimoPacoteLoraMs = millis(); desenharTelaAtual(); }
          else if (telaAtual == 8 && devSubPagina == 1) { mostrarTelaReset("RTC"); Wire.begin(); if(rtc.begin()){ falhaRTC=false; } desenharTelaAtual(); }
          else if (telaAtual == 9 && devSubPagina == 1) {
            if (devSDAcaoSelecionada == 0) { mostrarTelaReset("Cartao SD"); iniciarSDCard_Seguro(); }
            else if (devSDAcaoSelecionada == 1) { devSubPagina = 2; }
            desenharTelaAtual();
          } else if (telaAtual == 6) { 
            if (menuPadroesSelecionado == 0) { iniciarCalibracao(); }
            else if (menuPadroesSelecionado == 1) { iniciarResetPadroes(); }
          }
        } 
        else if (curEsq || curDir) {
            // Lógica de Navegação Padrão
            if (telaAtual == 1) {
                if (curDir) appSelecionado = (appSelecionado < 2) ? appSelecionado + 1 : 0;
                if (curEsq) appSelecionado = (appSelecionado > 0) ? appSelecionado - 1 : 2;
            }
            else if (telaAtual == 2) {
                if (curDir) dadosSubTela = (dadosSubTela < 2) ? dadosSubTela + 1 : 0;
                if (curEsq) { if (dadosSubTela == 0) telaAtual = 1; else dadosSubTela--; }
            }
            else if (telaAtual == 3) {
                if (curDir) menuAjustesSelecionado = (menuAjustesSelecionado < 2) ? menuAjustesSelecionado + 1 : 0;
                if (curEsq) telaAtual = 1; 
            }
            else if (telaAtual == 4 || telaAtual == 5 || telaAtual == 6) {
                  if (curEsq) telaAtual = 3; 
                  if (curDir) {
                    if(telaAtual == 4) devMenuSelecionado = (devMenuSelecionado < 3) ? devMenuSelecionado + 1 : 0;
                    if(telaAtual == 6) menuPadroesSelecionado = (menuPadroesSelecionado < 1) ? menuPadroesSelecionado + 1 : 0;
                  }
            }
            else if (telaAtual > 6) {
                if (curEsq) { if (devSubPagina > 0) devSubPagina--; else telaAtual = 4; } 
                if (curDir) {
                    if (devSubPagina == 0) devSubPagina = 1;
                    else if (telaAtual == 9 && devSubPagina == 1) devSDAcaoSelecionada = (devSDAcaoSelecionada < 1) ? devSDAcaoSelecionada + 1 : 0;
                }
            }
            else { if (curEsq) telaAtual = 1; }
            
            destravarSPI();
            desenharTelaAtual();
        }
    }
  }

  if (emProcessoDeCalibracao) {
    if (millis() - inicioCalibracaoMs < 60000UL) {
      desenharTelaCalibracao((float)(millis() - inicioCalibracaoMs) / 60000.0f);
    } else { finalizarCalibracao(); }
  }

  // Atualizações periódicas (Só se não estiver no popup)
  if (millis() - lastUpdateMs > 500 && !emProcessoDeCalibracao) {
      lastUpdateMs = millis();
      verificarSaudeModulos(); // Atualiza status
      destravarSPI();
      
      if (!falhaModuloVisivel) { // Só desenha tela normal se não tiver falha visível
          atualizarTela(); 
      }
      verificarAlertas(); // Som
  }

  if (millis() - ultimoTempoGravacaoSD > INTERVALO_GRAVACAO_SD) {
    ultimoTempoGravacaoSD = millis();
    gravarSD_Seguro(); 
  }
}

void receberLoraBinario() {
    int packetSize = LoRa.parsePacket();
    if (packetSize == sizeof(DadosSensores)) {
        LoRa.readBytes((uint8_t*)&dadosRecebidos, sizeof(DadosSensores));
        rssiLoRa = LoRa.packetRssi(); 
        ultimoPacoteLoraMs = millis();
        falhaLoRa = false; erroLoRa = "OK";

        ultimoPacoteLoRaBruto = String("I:") + String(dadosRecebidos.corrente, 2) + " T:" + String(dadosRecebidos.temperatura, 1);

        float r_bruto = sqrt(dadosRecebidos.ax*dadosRecebidos.ax + dadosRecebidos.ay*dadosRecebidos.ay + dadosRecebidos.az*dadosRecebidos.az);

        if (primeiraLeituraAposReset) {
            corrente = dadosRecebidos.corrente;
            temperatura = dadosRecebidos.temperatura;
            eixoX = dadosRecebidos.ax;
            eixoY = dadosRecebidos.ay;
            eixoZ = dadosRecebidos.az;
            vibracaoRMS = r_bruto;
            primeiraLeituraAposReset = false;
            corrMin = corrMax = corrente;
            tempMin = tempMax = temperatura;
        } else {
            corrente = (ALPHA * dadosRecebidos.corrente) + ((1.0 - ALPHA) * corrente);
            temperatura = (ALPHA * dadosRecebidos.temperatura) + ((1.0 - ALPHA) * temperatura);
            eixoX = (ALPHA * dadosRecebidos.ax) + ((1.0 - ALPHA) * eixoX);
            eixoY = (ALPHA * dadosRecebidos.ay) + ((1.0 - ALPHA) * eixoY);
            eixoZ = (ALPHA * dadosRecebidos.az) + ((1.0 - ALPHA) * eixoZ);
            vibracaoRMS = (ALPHA * r_bruto) + ((1.0 - ALPHA) * vibracaoRMS);
            
            if (corrente > corrMax) corrMax = corrente;
            if (corrente < corrMin) corrMin = corrente;
            if (temperatura > tempMax) tempMax = temperatura;
            if (temperatura < tempMin) tempMin = temperatura;
        }
        if(emProcessoDeCalibracao) atualizarSomaCalibracao();
        historicoX[indiceHistorico] = eixoX; historicoY[indiceHistorico] = eixoY;
        historicoZ[indiceHistorico] = eixoZ; historicoRMS[indiceHistorico] = vibracaoRMS;
        indiceHistorico = (indiceHistorico + 1) % NUM_PONTOS_GRAFICO;
        Serial.printf("RX;%.2f;%.1f;%.2f\n", corrente, temperatura, vibracaoRMS);
    }
}