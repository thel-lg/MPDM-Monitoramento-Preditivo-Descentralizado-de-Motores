#ifndef CALIBRACAO_H
#define CALIBRACAO_H

#include <Arduino.h>

// Variáveis externas que este arquivo precisa conhecer
extern Adafruit_ST7735 tft;
extern float corrente, vibracaoRMS, temperatura;
extern float padraoCorrente, padraoVibracao, padraoTemperatura;
extern bool padroesDefinidos, primeiraLeituraAposReset;
extern bool emProcessoDeCalibracao;
extern int telaAtual;
extern float tempMin, tempMax, corrMin, corrMax;
extern unsigned long inicioCalibracaoMs;
extern float somaCalibracaoI, somaCalibracaoV, somaCalibracaoT;
extern int contadorCalibracao;

// Protótipos de funções que este arquivo chama
void desenharTelaCalibracao(float percent);
void desenharTelaStatus(const char* linha1, const char* linha2, uint16_t cor);
void desenharTelaAtual();

// Limite mínimo de amostras para uma calibração ser válida
const int MINIMO_AMOSTRAS_CALIBRACAO = 30; 

void iniciarCalibracao() {
  if(emProcessoDeCalibracao) return;
  
  emProcessoDeCalibracao = true;
  inicioCalibracaoMs = millis();
  
  somaCalibracaoI = 0; somaCalibracaoV = 0; somaCalibracaoT = 0;
  contadorCalibracao = 0;
  // Não precisa mais resetar 'etapaAnimacao' nem desenhar a tela aqui.
}

void atualizarSomaCalibracao() {
  if(emProcessoDeCalibracao && (corrente > 0 || vibracaoRMS > 0 || temperatura > 0)){
      somaCalibracaoI += corrente;
      somaCalibracaoV += vibracaoRMS;
      somaCalibracaoT += temperatura;
      contadorCalibracao++;
  }
}



void finalizarCalibracao() {
  if (contadorCalibracao > MINIMO_AMOSTRAS_CALIBRACAO) {
    padraoCorrente = somaCalibracaoI / contadorCalibracao;
    padraoVibracao = somaCalibracaoV / contadorCalibracao;
    padraoTemperatura = somaCalibracaoT / contadorCalibracao;
    padroesDefinidos = true;
    primeiraLeituraAposReset = true;
    desenharTelaStatus("Padroes", "Definidos!", ST77XX_GREEN);
  } else {
    desenharTelaStatus("Falha:", "Poucos dados recebidos", COR_PERIGO);
  }
  
  delay(2500);
  emProcessoDeCalibracao = false;
  telaAtual = 3;
  desenharTelaAtual();
}

// EM CALIBRACAO.H: Substitua a função COMPLETA toggleModoGrafico()

void toggleModoGrafico() {
    padroesDefinidos = !padroesDefinidos;
    
    if (padroesDefinidos) {
        // MODO GRÁFICO ATIVADO
        // Se ativamos, forçamos um reset dos padrões para evitar o bug do alarme imediato.
        padraoCorrente = 0; padraoVibracao = 0; padraoTemperatura = 0;
        
        desenharTelaStatus("Grafico On", "Execute a Calibracao", COR_ACENTO);
        logarErro("Grafico ON. Necessaria Calibracao.");

    } else {
        // MODO GRÁFICO DESATIVADO
        padraoCorrente = 0; padraoVibracao = 0; padraoTemperatura = 0;
        desenharTelaStatus("Grafico", "Off", COR_PERIGO);
    }

    tempMin = 0; tempMax = 0; corrMin = 0; corrMax = 0;
    primeiraLeituraAposReset = true;
    
    delay(1500);
    telaAtual = 4;
}

void iniciarResetPadroes() {
    desenharTelaStatus("Padroes", "Resetados", COR_PERIGO);
    padraoCorrente = 0;
    padraoVibracao = 0;
    padraoTemperatura = 0;
    padroesDefinidos = false;
    tempMin = 0; tempMax = 0; corrMin = 0; corrMax = 0;
    primeiraLeituraAposReset = true;
    delay(2000);
    telaAtual = 6;
    desenharTelaAtual();
}

#endif