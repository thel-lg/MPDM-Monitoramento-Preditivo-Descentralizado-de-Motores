#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "bateria.h" 
#include "Cartao.h"

#define COR_FUNDO       ST77XX_BLACK
#define COR_PRINCIPAL   0x39E7
#define COR_ACENTO      0x05DF
#define COR_TEXTO       ST77XX_WHITE
#define COR_PERIGO      0xF9A7
#define COR_ALERTA      ST77XX_ORANGE
#define COR_PADRAO      ST77XX_YELLOW
#define COR_OK          ST77XX_GREEN
#define ST77XX_DARKGREY 0x2104

// === VARIÁVEIS GLOBAIS EXTERNAS (DECLARADAS NO .INO) ===
extern Adafruit_ST7735 tft;
extern RTC_DS3231 rtc;
extern int appSelecionado, menuAjustesSelecionado, menuPadroesSelecionado, devMenuSelecionado, devSubPagina, devSDAcaoSelecionada, scrollIndex;
extern bool falhaRTC, falhaSD, falhaLoRa, padroesDefinidos;
extern bool alertaAmareloAtivo, alertaVermelhoAtivo;
extern float corrente, padraoCorrente, vibracaoRMS, padraoVibracao, temperatura, padraoTemperatura;
extern float corrMin, corrMax, tempMin, tempMax;
extern float historicoX[], historicoY[], historicoZ[], historicoRMS[];
extern int indiceHistorico;
extern String erroRTC, erroSD, erroLoRa, ultimoPacoteLoRaBruto;
extern bool emProcessoDeCalibracao;

// === PROTÓTIPOS ===
void desenhaIconeAlertaExclamacao(int16_t x, int16_t y, uint16_t cor);
void desenhaIconeMonitor(int16_t x, int16_t y, uint16_t cor);
void desenhaIconeAjustes(int16_t x, int16_t y, uint16_t cor);
void iniciarDisplay();
void telaInicialProfissional(const char* msg, float progresso);
void telaDesligando();
void desenharCabecalho();
void desenharHomeScreen();
void desenharTelaNotificacoes();
void desenharTelaAjustes();
void desenharTelaPadroes();
void desenharTelaDevMode();
void desenharTelaDev_LoRa();
void desenharTelaDev_RTC();
void desenharTelaDev_SD();
void desenharTextoComQuebra(int16_t x, int16_t y, int16_t maxWidth, String text);
void desenharTelaSobre();
void desenharPopupFalha();
void desenharTelaAtual();
void atualizarTela();
void desenharTelaCalibracao(float percent);
void desenharTelaStatus(const char* linha1, const char* linha2, uint16_t cor);
void mostrarTelaReset(const char* modulo);
void desenharBateria();
void desenharTelaDados();
void desenharTelaDados_Live();
void desenharTelaDados_Eixos();
void desenharTelaDados_RMS_Extremos();
void desenharGraficoPequeno(int x, int y, int w, int h, float dados[], int n, const char* titulo, uint16_t cor);
// ... (lista de protótipos)
void desenharTelaDados_RMS_Extremos();
void desenharGraficoPequeno(int x, int y, int w, int h, float dados[], int n, const char* titulo, uint16_t cor);
void desenharTelaListaArquivosSD(); // <-- ADICIONE ESTA LINHA
// ...
// === IMPLEMENTAÇÃO ===

void iniciarDisplay() {
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(0);
  tft.fillScreen(COR_FUNDO);
  #if ESP_IDF_VERSION_MAJOR >= 5
    ledcAttach(TFT_LED, 5000, 8);
    ledcWrite(TFT_LED, 140);
  #else
    ledcSetup(0, 5000, 8);
    ledcAttachPin(TFT_LED, 0);
    ledcWrite(0, 140);
  #endif
}

void telaInicialProfissional(const char* msg, float progresso) {
  tft.fillScreen(COR_FUNDO);
  tft.drawRoundRect(15, 40, 100, 40, 8, COR_PRINCIPAL);
  tft.setTextSize(3);
  tft.setTextColor(COR_ACENTO);
  tft.setCursor(28, 50);
  tft.print("MPDM");
  int barraX = 14, barraY = 120, barraW = 100, barraH = 10;
  tft.drawRect(barraX, barraY, barraW, barraH, COR_PRINCIPAL);
  tft.fillRect(barraX + 2, barraY + 2, (int)(progresso * (barraW - 4)), barraH - 4, COR_ACENTO);
  tft.setTextSize(1);
  tft.setTextColor(COR_TEXTO);
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((128 - w) / 2, barraY + 15);
  tft.print(msg);
}

void desenhaIconeAlertaExclamacao(int16_t x, int16_t y, uint16_t cor) {
    tft.fillRoundRect(x + 13, y + 2, 6, 18, 3, cor);
    tft.fillCircle(x + 16, y + 26, 3, cor);
}

void desenhaIconeMonitor(int16_t x, int16_t y, uint16_t cor) {
    tft.fillRect(x + 6, y + 18, 6, 12, cor);
    tft.fillRect(x + 13, y + 10, 6, 20, cor);
    tft.fillRect(x + 20, y + 14, 6, 16, cor);
    tft.drawFastHLine(x + 2, y + 31, 28, cor);
}

void desenhaIconeAjustes(int16_t x, int16_t y, uint16_t cor) {
    tft.fillCircle(x + 16, y + 16, 10, cor);
    tft.fillCircle(x + 16, y + 16, 6, COR_FUNDO);
    for (int i = 0; i < 8; i++) {
        float angle = i * (PI / 4);
        int16_t x1 = 16 + 8 * cos(angle);
        int16_t y1 = 16 + 8 * sin(angle);
        int16_t x2 = 16 + 14 * cos(angle);
        int16_t y2 = 16 + 14 * sin(angle);
        tft.drawLine(x + x1, y + y1, x + x2, y + y2, cor);
        tft.drawLine(x + x1+1, y + y1, x + x2+1, y + y2, cor);
    }
}

void desenharCabecalho() {
  tft.fillRect(0, 0, 128, 18, COR_PRINCIPAL);
  tft.setTextColor(COR_TEXTO);
  tft.setTextSize(1);
  tft.setCursor(5, 5);
  if (!falhaRTC) {
    DateTime now = rtc.now();
    tft.printf("%02d:%02d  %02d/%02d", now.hour(), now.minute(), now.day(), now.month());
  } else {
    tft.print("--:--  --/--");
  }

  // Círculo de notificação menor e com cores dinâmicas
  if (alertaVermelhoAtivo) {
    tft.fillCircle(88, 9, 4, COR_PERIGO); // Círculo Vermelho (Raio 4)
  } else if (alertaAmareloAtivo) {
    tft.fillCircle(88, 9, 4, COR_ALERTA); // Círculo Amarelo (Raio 4)
  }

  desenharBateria();
}

void desenharHomeScreen() {
  int icon_y = 45;
  int icon_x[] = {12, 48, 84};
  
  desenhaIconeAlertaExclamacao(icon_x[0], icon_y, (appSelecionado == 0) ? COR_ACENTO : COR_PRINCIPAL);
  desenhaIconeMonitor(icon_x[1], icon_y, (appSelecionado == 1) ? COR_ACENTO : COR_PRINCIPAL);
  desenhaIconeAjustes(icon_x[2], icon_y, (appSelecionado == 2) ? COR_ACENTO : COR_PRINCIPAL);

  tft.fillCircle(icon_x[appSelecionado] + 16, icon_y + 32 + 8, 4, COR_ACENTO);
  
  tft.setTextSize(2);
  tft.setTextColor(COR_TEXTO);
  const char* nomes[] = {"Alertas", "Dados", "Ajustes"};
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(nomes[appSelecionado], 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((128 - w) / 2, 110);
  tft.print(nomes[appSelecionado]);
  
  tft.fillRect(0, 150, 128, 10, COR_PRINCIPAL);
  tft.setTextSize(1);
  tft.setTextColor(COR_TEXTO);
  tft.setCursor(5, 151); tft.print("< NAVEGAR >");
  tft.setCursor(110, 151); tft.print("OK");
}

void desenharTelaNotificacoes() {
    tft.setTextColor(COR_TEXTO); tft.setTextSize(2);
    tft.setCursor((128 - 7*12)/2, 25); tft.print("Alertas");
    tft.drawFastHLine(0, 45, 128, COR_PRINCIPAL);
    int y = 55; bool semProblemas = true; tft.setTextSize(1);
    
    // --- FALHAS DE MÓDULOS (Criticas e Operacionais - Usando Strings curtas) ---
    if (falhaRTC) { 
        tft.setTextColor(COR_PERIGO); 
        tft.setCursor(5, y); 
        tft.printf("- RTC: %s", erroRTC.c_str()); // Ex: - RTC: I2C FAIL
        y += 12; 
        semProblemas = false; 
    }
    if (falhaSD) { 
        tft.setTextColor(COR_PERIGO); 
        tft.setCursor(5, y); 
        tft.printf("- SD: %s", erroSD.c_str()); // Ex: - SD: MOUNT FAIL
        y += 12; 
        semProblemas = false; 
    }
    if (falhaLoRa) { 
        tft.setTextColor(COR_PERIGO); 
        tft.setCursor(5, y); 
        tft.printf("- LoRa: %s", erroLoRa.c_str()); // Ex: - LoRa: NO SIGNAL ou HARDWARE FAIL
        y += 12; 
        semProblemas = false; 
    }
    
    if(padroesDefinidos) {
        if (corrente > padraoCorrente * 1.2) { tft.setTextColor(ST77XX_ORANGE); tft.setCursor(5, y); tft.println("-Corrente alta"); y += 12; semProblemas = false; }
        if (temperatura > padraoTemperatura * 1.2) { tft.setTextColor(ST77XX_ORANGE); tft.setCursor(5, y); tft.println("-Temperatura alta"); y += 12; semProblemas = false; }
        if (vibracaoRMS > padraoVibracao * 1.2) { tft.setTextColor(ST77XX_ORANGE); tft.setCursor(5, y); tft.println("-Vibracao alta"); y += 12; semProblemas = false; }
    }

    if (semProblemas) {
        tft.setTextColor(ST77XX_GREEN);
        tft.setTextSize(2);
        const char* msgOk = "Tudo OK";
        int16_t x1, y1; uint16_t w, h;
        tft.getTextBounds(msgOk, 0, 0, &x1, &y1, &w, &h);
        tft.setCursor((128 - w) / 2, 80);
        tft.println(msgOk);
    }

    tft.fillRect(0, 150, 128, 10, COR_PRINCIPAL); tft.setTextColor(COR_TEXTO); tft.setTextSize(1);
    tft.setCursor(5, 151); tft.print("<");
}

void desenharTelaAjustes() {
  tft.setTextColor(COR_TEXTO); tft.setTextSize(2);
  tft.setCursor((128 - 7*12)/2, 25); tft.print("Ajustes");
  tft.drawFastHLine(0, 45, 128, COR_PRINCIPAL);
  const char* itensMenu[] = {"Padroes", "Sobre o Sistema", "Modo Desenvolvedor"};
  tft.setTextSize(1);
  for(int i=0; i<3; i++) {
    int yPos = 62 + (i*25);
    if(i == menuAjustesSelecionado) {
      tft.fillRect(5, yPos - 5, 118, 20, ST77XX_WHITE);
      tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
    } else {
      tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    }
    tft.setCursor(10, yPos); // Centralizado na esquerda
    tft.print(itensMenu[i]);
  }
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.fillRect(0, 150, 128, 10, COR_PRINCIPAL); tft.setTextColor(COR_TEXTO);
  tft.setCursor(5, 151); tft.print("< Voltar");
  tft.setCursor(65, 151); tft.print("NAV >");
  tft.setCursor(110, 151); tft.print("OK");
}


void desenharTelaPadroes() {
  tft.setTextColor(COR_TEXTO); tft.setTextSize(2);
  tft.setCursor((128 - 7*12)/2, 25); tft.print("Padroes");
  tft.drawFastHLine(0, 45, 128, COR_PRINCIPAL);
  const char* itensMenu[] = {"Iniciar Calibracao", "Resetar Padroes"};
  tft.setTextSize(1);
  for(int i=0; i<2; i++) {
    int yPos = 70 + (i*30);
    int16_t x1, y1; uint16_t w, h;
    tft.getTextBounds(itensMenu[i], 0, 0, &x1, &y1, &w, &h);
    if(i == menuPadroesSelecionado) {
      tft.fillRect( (128-w)/2 - 5, yPos - 5, w+10, 20, ST77XX_WHITE);
      tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
    } else {
      tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    }
    tft.setCursor((128 - w) / 2, yPos);
    tft.print(itensMenu[i]);
  }
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.fillRect(0, 150, 128, 10, COR_PRINCIPAL); tft.setTextColor(COR_TEXTO);
  tft.setCursor(5, 151); tft.print("< Voltar");
  tft.setCursor(65, 151); tft.print("NAV >");
  tft.setCursor(110, 151); tft.print("OK");
}

void desenharTelaDados() {
    // A habilitação para ver as telas agora é controlada por "padroesDefinidos"
    // que é ativado/desativado pelo botão "Grafico" no menu DEV.
    if (!padroesDefinidos) {
        desenharTelaDados_Live(); // Mostra apenas a tela principal se os padrões não estiverem definidos
    } else {
        // Nova ordem: 0 = Ao Vivo, 1 = RMS/Extremos, 2 = Eixos
        switch(dadosSubTela) {
            case 0: desenharTelaDados_Live(); break;
            case 1: desenharTelaDados_RMS_Extremos(); break;
            case 2: desenharTelaDados_Eixos(); break;
        }
    }
}

void desenharTelaDados_Live() {
    tft.setTextColor(ST77XX_CYAN, COR_FUNDO); tft.setTextSize(2); tft.setCursor(5, 30); tft.printf("%.2f A", corrente);
    tft.setTextColor(COR_PADRAO, COR_FUNDO); tft.setTextSize(1); tft.setCursor(5, 48); tft.printf("Padrao: %.2f", padraoCorrente);
    tft.setTextColor(ST77XX_GREEN, COR_FUNDO); tft.setTextSize(2); tft.setCursor(5, 70); tft.printf("%.2f m/s", vibracaoRMS);
    tft.setTextColor(COR_PADRAO, COR_FUNDO); tft.setTextSize(1); tft.setCursor(5, 88); tft.printf("Padrao: %.2f", padraoVibracao);
    tft.setTextColor(ST77XX_RED, COR_FUNDO); tft.setTextSize(2); tft.setCursor(5, 110); tft.printf("%.1f C", temperatura);
    tft.setTextColor(COR_PADRAO, COR_FUNDO); tft.setTextSize(1); tft.setCursor(5, 128); tft.printf("Padrao: %.1f", padraoTemperatura);
    
    tft.fillRect(0, 150, 128, 10, COR_PRINCIPAL); tft.setTextColor(COR_TEXTO); tft.setTextSize(1);
    tft.setCursor(5, 151); tft.print("< Voltar");
    if(padroesDefinidos) {
        tft.setCursor(115, 151); tft.print(">");
    }
}

// Função 1
void desenharTelaDados_Eixos() {
    tft.fillScreen(COR_FUNDO); 
    desenharCabecalho(); 

    const int GRAFICO_H = 28; // Altura do gráfico
    const char* UNIDADE = " (m/s)"; 
    
    // Eixo X: Y do Gráfico = 30 (Título em 20) -> Centraliza o bloco na área acima de Y=150
    desenharGraficoPequeno(14, 30, 100, GRAFICO_H, historicoX, NUM_PONTOS_GRAFICO, ("Eixo X" + String(UNIDADE)).c_str(), ST77XX_RED);
    
    // Eixo Y: Y do Gráfico = 70 (Título em 60)
    desenharGraficoPequeno(14, 70, 100, GRAFICO_H, historicoY, NUM_PONTOS_GRAFICO, ("Eixo Y" + String(UNIDADE)).c_str(), ST77XX_GREEN);

    // Eixo Z: Y do Gráfico = 110 (Título em 100)
    desenharGraficoPequeno(14, 110, 100, GRAFICO_H, historicoZ, NUM_PONTOS_GRAFICO, ("Eixo Z" + String(UNIDADE)).c_str(), ST77XX_CYAN);

    // Rodapé
    tft.fillRect(0, 150, 128, 10, COR_PRINCIPAL); tft.setTextColor(COR_TEXTO); tft.setTextSize(1);
    tft.setCursor(5, 151); tft.print("<");
    tft.setCursor(115, 151); tft.print(">");
}

// EM Display.h: Substitua a função COMPLETA desenharTelaDados_RMS_Extremos()

// DISPLAY.h

void desenharTelaDados_RMS_Extremos() {
    const int GRAFICO_H = 35;
    const int GRAFICO_Y = 40; 
    
    // Gráfico (Título em Y=30, Caixa em Y=40. Gráfico termina em 75)
    desenharGraficoPequeno(14, GRAFICO_Y, 100, GRAFICO_H, historicoRMS, NUM_PONTOS_GRAFICO, "Vibracao RMS (m/s)", ST77XX_YELLOW);
    
    tft.setTextSize(1);
    
    // --- LÓGICA DE ALINHAMENTO E IMPRESSÃO (CORRIGIDA) ---
    
    // Y inicial após o gráfico (Ex: 85)
    int y_pos = 85; 
    const int LINE_HEIGHT = 12; 
    
    // Corrente: Linha 1 (Corr Min)
    tft.setCursor(10, y_pos); 
    // **CORREÇÃO CRÍTICA:** Seta a cor do texto e a cor do fundo para apagar o texto anterior
    tft.setTextColor(COR_TEXTO, COR_FUNDO); 
    tft.printf("Corr Min: %.2f A", corrMin);
    
    // Corrente: Linha 2 (Corr Max)
    y_pos += LINE_HEIGHT; 
    tft.setCursor(10, y_pos); 
    tft.setTextColor(COR_TEXTO, COR_FUNDO); 
    tft.printf("Corr Max: %.2f A", corrMax); 
    
    // Espaçamento EXTRA entre Corrente e Temperatura
    y_pos += LINE_HEIGHT + 10; 
    
    // Temperatura: Linha 1 (Temp Min)
    tft.setCursor(10, y_pos); 
    tft.setTextColor(COR_TEXTO, COR_FUNDO); 
    tft.printf("Temp Min: %.1f C", tempMin); 
    
    // Temperatura: Linha 2 (Temp Max)
    y_pos += LINE_HEIGHT; 
    tft.setCursor(10, y_pos); 
    tft.setTextColor(COR_TEXTO, COR_FUNDO); 
    tft.printf("Temp Max: %.1f C", tempMax); 

    // Rodapé (sem fundo, só a cor do texto)
    tft.fillRect(0, 150, 128, 10, COR_PRINCIPAL); 
    tft.setTextColor(COR_TEXTO); 
    tft.setTextSize(1);
    tft.setCursor(5, 151); tft.print("<");
    tft.setCursor(115, 151); tft.print(">");
}

void desenharGraficoPequeno(int x, int y, int w, int h, float dados[], int n, const char* titulo, uint16_t cor) {
    tft.drawRect(x, y, w, h, COR_PRINCIPAL);
    tft.fillRect(x+1, y+1, w-2, h-2, COR_FUNDO);
    tft.setTextColor(COR_TEXTO, COR_FUNDO); tft.setTextSize(1);
    tft.setCursor(x+2, y-10); // Título 10 pixels ACIMA da caixa do gráfico
    tft.print(titulo); 

    // 1. CÁLCULO DE MIN/MAX
    float minVal = dados[0], maxVal = dados[0];
    for (int i = 1; i < n; i++) {
        if (dados[i] < minVal) minVal = dados[i];
        if (dados[i] > maxVal) maxVal = dados[i];
    }
    
    float range;
    float offset = 0; // Inicializa offset para o escopo correto

    // Calcula o valor médio para todos os gráficos (usado para offset/centralização)
    float soma = 0;
    for (int i = 0; i < n; i++) { soma += dados[i]; }
    offset = soma / n; // Valor médio (DC)
    
    // --- LÓGICA DE DIMENSIONAMENTO E RANGE ---
    if (strstr(titulo, "RMS")) {
        // --- RMS: Centralizar o ruído (offset) na metade da caixa ---
        
        // 1. Calcula a variação máxima (pico - repouso)
        float maxVariacao = maxVal - offset; 
        
        // 2. Define a metade do range com base na maior variação
        float halfRange = max(maxVariacao, 0.2f); // Garante 0.2 como range mínimo para o ruído
        
        // 3. O range total é o dobro da variação (para simetria visual)
        range = 2.0f * halfRange; 
        
        // 4. Define minVal e maxVal para que o offset (o ruído estável) caia no meio
        minVal = offset - halfRange;
        maxVal = offset + halfRange;
        
        // Garante que o RMS não mapeie abaixo de zero absoluto (se offset < halfRange)
        if (minVal < 0) minVal = 0;

        // Se o minVal for forçado a zero, o range precisa ser reajustado
        if (minVal == 0) range = maxVal;


    } else if (strstr(titulo, "Corrente")) {
        // --- CORRENTE: Apenas garantir o range mínimo de 0.2 acima do zero
        minVal = 0.0f; 
        range = maxVal - minVal;
        if (range < 0.2) range = 0.2;
        
    } else {
        // --- EIXOS X, Y, Z (Vibração AC) ---
        
        // Recalcula o max e min em relação ao offset (para que os dados mapeáveis sejam AC)
        float minVariacao = minVal - offset;
        float maxVariacao = maxVal - offset;
        
        // Centraliza em torno de zero (a vibração AC)
        float absMax = max(abs(minVariacao), abs(maxVariacao));
        absMax = max(absMax, 0.2f); 

        minVal = -absMax;
        maxVal = absMax;
        range = 2.0f * absMax; 
    }

    int lastX = x + 1;
    int lastY = y + h - 2; 
    int startIdx = (indiceHistorico + (n - (w-2)) ) % n;
    
    // 2. DESENHO DO GRÁFICO
    for (int i = 0; i < w-2; i++) {
        int pontoAtual = (startIdx + i) % n;
        int currentX = x + 1 + i;
        
        float dado_mapeavel = dados[pontoAtual];
        
        // Se for Eixo, remove o offset DC (Gravidade, ou ruído de centro) antes de mapear
        if (! (strstr(titulo, "RMS") || strstr(titulo, "Corrente")) ) {
            dado_mapeavel -= offset; 
        } 
        // Para RMS/Corrente, a lógica de mapeamento é feita diretamente com os valores brutos e os min/max/range calculados acima.

        // Mapeamento: (Dado - Min) / Range * Altura
        float normalizedVal = (dado_mapeavel - minVal) / range;
        
        int currentY = y + h - 2 - (int)(normalizedVal * (h - 4));
        
        if (currentY < y + 2) currentY = y + 2; 
        if (currentY > y + h - 2) currentY = y + h - 2; 

        if (i > 0) {
            tft.drawLine(lastX, lastY, currentX, currentY, cor);
        }
        lastX = currentX;
        lastY = currentY; 
    }
}
// Esta função substitui completamente a versão antiga
void desenharTextoComQuebra(int16_t x, int16_t y, int16_t maxWidth, String text) {
  int16_t currentX = x;
  int16_t currentY = y;
  String word = "";
  String line = "";

  // Adiciona um espaço no final para garantir que a última palavra seja processada
  text += " "; 

  for (int i = 0; i < text.length(); i++) {
    char c = text.charAt(i);

    if (c == ' ' || c == '\n') {
      int16_t x1, y1;
      uint16_t w, h;

      // Mede o tamanho da linha ATUAL + a próxima palavra
      tft.getTextBounds(line + word, 0, 0, &x1, &y1, &w, &h);

      if (w > maxWidth) { // Se a linha com a nova palavra estoura a largura
        tft.setCursor(currentX, currentY);
        tft.print(line); // Imprime a linha antiga
        currentY += h + 2; // Move o Y para baixo (altura da fonte + 2 pixels de espaçamento)
        line = word + " "; // A nova linha começa com a palavra atual
      } else {
        line += word + " "; // Adiciona a palavra à linha atual
      }
      word = ""; // Limpa a palavra

      if (c == '\n') { // Se o caractere for uma quebra de linha explícita
          tft.setCursor(currentX, currentY);
          tft.print(line);
          currentY += h + 2;
          line = "";
      }

    } else {
      word += c; // Monta a palavra caractere por caractere
    }
  }

  // Imprime qualquer texto restante na última linha
  if (line.length() > 0) {
    tft.setCursor(currentX, currentY);
    tft.print(line);
  }
}
void desenharTelaDevMode() {
  tft.setTextColor(COR_TEXTO); tft.setTextSize(2);
  tft.setCursor((128 - 10*12)/2, 25); tft.print("Modo DEV");
  tft.drawFastHLine(0, 45, 128, COR_PRINCIPAL);
  
  const char* modulos[] = {"LoRa", "RTC", "Cartao SD", "Grafico"}; // Renomeado
  
  tft.setTextSize(1);
  for(int i=0; i<4; i++) {
    int yPos = 60 + (i*24);
    String textoItem = String(modulos[i]);
    if (i == 3) {
        textoItem += padroesDefinidos ? " [ON]" : " [OFF]";
    }

    if(i == devMenuSelecionado) {
        tft.fillRect(5, yPos - 5, 118, 20, ST77XX_WHITE);
        tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
    } else {
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    }
    tft.setCursor(10, yPos); // Alinhado à esquerda
    tft.print(textoItem);
  }
  
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.fillRect(0, 150, 128, 10, COR_PRINCIPAL); tft.setTextColor(COR_TEXTO);
  tft.setCursor(5, 151); tft.print("< Voltar");
  tft.setCursor(65, 151); tft.print("NAV >");
  tft.setCursor(110, 151); tft.print("OK");
}
void desenharTelaDev_LoRa() {
  tft.setTextColor(COR_TEXTO); tft.setTextSize(2);
  tft.setCursor(5, 25); tft.print("LoRa");
  tft.drawFastHLine(0, 45, 128, COR_PRINCIPAL);
  tft.setTextSize(1);

  if (devSubPagina == 0) { // Página 1: Status Reorganizado
    tft.setCursor(5, 60); tft.printf("Frequencia: 433 MHz");
    tft.setCursor(5, 75); tft.printf("Ultimo Sinal:ha %lus", (millis() - ultimoPacoteLoraMs)/1000);
    
    tft.setCursor(5, 90); tft.print("Status: ");
    tft.setTextColor(falhaLoRa ? COR_PERIGO : COR_OK);
    // A função de quebra de texto agora gerencia a posição para não sobrepor
    desenharTextoComQuebra(tft.getCursorX(), tft.getCursorY(), 118, erroLoRa);

  } else { // Página 2: Pacote com layout ajustado
    tft.setCursor(5, 55); tft.println("Ultimo recebimento:");
    
    // Retângulo movido para baixo
    tft.drawRect(2, 72, 124, 45, COR_PRINCIPAL);
    tft.setTextColor(COR_ACENTO);
    tft.setTextSize(0); // Fonte menor para o pacote
    
    // Desenha com margem interna de 5 pixels
    desenharTextoComQuebra(2 + 5, 72 + 5, 124 - 10, ultimoPacoteLoRaBruto);
    
    tft.setTextSize(1); // Restaura tamanho da fonte
    tft.fillRect(14, 125, 100, 20, ST77XX_WHITE);
    tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
    tft.setCursor(20, 131); tft.print("Resetar Modulo");
  }

  tft.fillRect(0, 150, 128, 10, COR_PRINCIPAL); tft.setTextColor(COR_TEXTO);
  tft.setCursor(5, 151); tft.print("< Voltar");
  tft.setCursor(115, 151); tft.print(">");
}

void desenharTelaDev_RTC() {
  tft.setTextColor(COR_TEXTO); tft.setTextSize(2);
  tft.setCursor(5, 25); tft.print("RTC");
  tft.drawFastHLine(0, 45, 128, COR_PRINCIPAL);
  tft.setTextSize(1);
  if (devSubPagina == 0) { // Página 1: Status
    tft.setCursor(5, 60); tft.print("Status:");
    tft.setTextColor(falhaRTC ? COR_PERIGO : COR_OK); tft.print(erroRTC);
    tft.setTextColor(COR_TEXTO);
    if(!falhaRTC){
      DateTime now = rtc.now();
      tft.setCursor(5, 80); tft.printf("Data: %02d/%02d/%04d", now.day(), now.month(), now.year());
      tft.setCursor(5, 95); tft.printf("Hora: %02d:%02d:%02d", now.hour(), now.minute(), now.second());
    }
  } else { // Página 2: Ação
    tft.setTextSize(1);
    tft.fillRect(14, 120, 100, 20, ST77XX_WHITE);
    tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
    tft.setCursor(20, 126); tft.print("Resetar Modulo");
  }
  tft.fillRect(0, 150, 128, 10, COR_PRINCIPAL); tft.setTextColor(COR_TEXTO);
  tft.setCursor(5, 151); tft.print("<");
  tft.setCursor(115, 151); tft.print(">");
}

void desenharTelaDev_SD() {
    if (devSubPagina == 2) {
        desenharTelaListaArquivosSD();
        return;
    }

    tft.setTextColor(COR_TEXTO); tft.setTextSize(2);
    tft.setCursor(5, 25); tft.print("Cartao SD");
    tft.drawFastHLine(0, 45, 128, COR_PRINCIPAL);
    tft.setTextSize(1);
    if (devSubPagina == 0) { // Página 1: Info
        if(falhaSD) {
            tft.setTextColor(COR_PERIGO);
            tft.setCursor(5,60);
            desenharTextoComQuebra(5, 60, 118, "Falha no cartao: " + erroSD);
        } else {
            tft.setTextColor(COR_OK);
            tft.setCursor(5, 60); tft.print("Cartao SD conectado.");
            tft.setTextColor(COR_TEXTO);
            tft.setCursor(5, 80); tft.printf("Tipo: %d", SD.cardType());
            tft.setCursor(5, 95); tft.printf("Tamanho: %lluMB", SD.cardSize()/(1024*1024));
        }
    } else if (devSubPagina == 1) { // Página 2: Ações
        const char* acoes[] = {"Resetar Modulo", "Listar Arquivos"};
        for(int i=0; i<2; i++){
            int yPos = 70 + (i*30);
            if(i == devSDAcaoSelecionada) {
                tft.fillRect(14, yPos-5, 100, 20, ST77XX_WHITE);
                tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
            } else {
                tft.setTextColor(ST77XX_WHITE, COR_FUNDO);
            }
            tft.setCursor(20, yPos); tft.print(acoes[i]);
        }
    }
    tft.fillRect(0, 150, 128, 10, COR_PRINCIPAL); tft.setTextColor(COR_TEXTO);
    tft.setCursor(5, 151); tft.print("<Voltar");
    tft.setCursor(115, 151); tft.print(">");
}

void desenharTelaSobre() {
  tft.setTextColor(COR_ACENTO); tft.setTextSize(2);
  tft.setCursor((128-4*12)/2, 25); tft.print("MPDM");
  tft.drawFastHLine(0, 45, 128, COR_PRINCIPAL);
  tft.setTextSize(0.7);
  tft.setTextColor(COR_TEXTO);
  tft.setCursor(5, 60); tft.println("Sistema de Monitora-mento Preditivo, ");
  tft.setCursor(5, 77); tft.println("Desenvolvido para o");
  tft.setCursor(5, 87); tft.println("TCC, feito por:");
  tft.setCursor(20, 97); tft.println("Lucas Santos,");
  tft.setCursor(20, 107); tft.println("Gustavo Santana,");
  tft.setCursor(20, 117); tft.println("Nicole Silva,");
  tft.setCursor(20, 127); tft.println("Polyanna Borges.");
  tft.fillRect(0, 150, 128, 10, COR_PRINCIPAL); tft.setTextColor(COR_TEXTO);
  tft.setCursor(5, 151); tft.print("< Voltar");
}

void desenharPopupFalha() {
    // LIMPEZA DE TELA (Corrige o bug visual)
    tft.fillScreen(COR_FUNDO);
    
    // Desenha a caixa de fundo
    tft.fillRect(10, 20, 108, 120, ST77XX_DARKGREY); 
    tft.drawRect(10, 20, 108, 120, COR_PERIGO);
    
    // Título
    tft.setTextColor(COR_PERIGO); tft.setTextSize(2);
    tft.setCursor((128-5*12)/2, 30); tft.println("FALHA");
    
    tft.drawFastHLine(15, 48, 98, COR_PERIGO);
    
    tft.setTextSize(1); tft.setTextColor(COR_TEXTO);
    int y = 60;

    // Lista os erros ativos
    if (falhaLoRa) { 
        tft.setCursor(15, y); tft.print("-LoRa: SEM SINAL"); 
        y += 12; 
    }
    
    if (falhaRTC) { 
        tft.setCursor(15, y); tft.print("-RTC: ERRO"); 
        y += 12; 
    }
    
    if (falhaSD) { 
        tft.setCursor(15, y); tft.print("-SD: REMOVIDO"); 
        y += 12; 
    }

    // Botão de confirmação
    tft.fillRect(15, 115, 98, 20, COR_PRINCIPAL); 
    tft.setTextColor(COR_TEXTO); tft.setTextSize(1);
    tft.setCursor((128 - 13*6)/2, 121); tft.println("Pressione OK");
}

void desenharTelaListaArquivosSD() {
    // Cabeçalho
    tft.setTextColor(COR_TEXTO); tft.setTextSize(2);
    tft.setCursor(5, 25); tft.print("Arquivos");
    tft.drawFastHLine(0, 45, 128, COR_PRINCIPAL);

    // Corpo (Lista)
    tft.setTextSize(1);
    tft.setTextColor(COR_ACENTO);
    
    // Chama a função do Cartao.h que lê o SD
    String lista = listarArquivos(); 
    
    // Usa sua função de quebra de texto para exibir a lista
    desenharTextoComQuebra(5, 55, 120, lista);
    
    // Rodapé
    tft.fillRect(0, 150, 128, 10, COR_PRINCIPAL); 
    tft.setTextColor(COR_TEXTO); tft.setTextSize(1);
    tft.setCursor(5, 151); tft.print("< Voltar");
}
// DISPLAY.h

void atualizarTela() { 
    
    // Se o sistema NÃO está em Calibração E NÃO está com Pop-up visível,
    // atualizamos o CONTEÚDO principal da tela (corpo e dados).
    if (!emProcessoDeCalibracao && !falhaModuloVisivel) { 
        
        // Lógica de Redesenho Rápido para o conteúdo principal
        if (telaAtual == 2) { 
            desenharTelaDados(); 
        } 
        else if (telaAtual == 8 && devSubPagina == 1) { 
            desenharTelaDev_RTC(); 
        }
        else if (telaAtual == 4 || (telaAtual > 6 && devSubPagina == 0) ) { 
            /* Não redesenha no loop rápido (telas de menu estáticas) */ 
        }
        else { 
            desenharTelaAtual(); // Redesenha a tela completa (Home, Ajustes, Notificações, etc.)
        }
    }
    
    // [CORREÇÃO FINAL DO RELÓGIO]: O Cabeçalho (Relógio/Bateria/Alertas) DEVE SER ATUALIZADO SEMPRE!
    desenharCabecalho(); 
}

// Esta função deve estar no seu arquivo Display.h
void desenharTelaCalibracao(float percent) {
  tft.fillScreen(COR_FUNDO);
  tft.setTextColor(COR_ACENTO); tft.setTextSize(2);
  const char* msg = "Calibrando...";
  int16_t x1, y1; uint16_t w, h;
  tft.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((128 - w) / 2, 60);
  tft.println(msg);
  
  int barraX = 14, barraY = 120, barraW = 100, barraH = 15;
  tft.drawRect(barraX, barraY, barraW, barraH, ST77XX_WHITE);
  tft.fillRect(barraX + 2, barraY + 2, (int)(percent * (barraW - 4)), barraH - 4, ST77XX_GREEN);
  
  tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
  String percentStr = String((int)(percent * 100)) + "%";
  tft.getTextBounds(percentStr, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((128 - w) / 2, barraY - 12);
  tft.print(percentStr);
}

void desenharTelaStatus(const char* linha1, const char* linha2, uint16_t cor) {
  tft.fillScreen(COR_FUNDO);
    
    // --- LINHA 1 (Título Principal) ---
    tft.setTextColor(cor); 
    tft.setTextSize(2); // Tamanho 2 para a primeira linha
    int16_t x1, y1; uint16_t w, h;
    tft.getTextBounds(linha1, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((128-w)/2, 60); 
    tft.println(linha1);

    tft.setTextColor(cor); // Usa branco para a instrução (ou a cor que preferir)
    tft.setTextSize(1); // [CORRIGIDO] Reduz o tamanho da fonte para caber na tela
    tft.getTextBounds(linha2, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((128-w)/2, 85); 
    tft.println(linha2);
}

void mostrarTelaReset(const char* modulo) {
    tft.fillScreen(COR_FUNDO);
    tft.setTextColor(COR_ACENTO);
    tft.setTextSize(2);
    
    int16_t x1, y1; uint16_t w, h;
    const char* msg1 = "Resetando";
    tft.getTextBounds(msg1, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((128 - w) / 2, 60);
    tft.println(msg1);

    tft.setTextSize(1);
    tft.setTextColor(COR_TEXTO);
    tft.getTextBounds(modulo, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((128 - w) / 2, 85);
    tft.println(modulo);
    
    delay(2000);
}

void telaDesligando() {
    tft.fillScreen(COR_FUNDO);
    tft.setTextColor(COR_PADRAO);
    tft.setTextSize(2);
    int16_t x1, y1;
    uint16_t w, h;
    const char* msg = "Desligando";
    tft.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((128 - w) / 2, 70);
    tft.println(msg);
    delay(1200);
}

void desenharTelaAtual() {
  tft.fillScreen(COR_FUNDO);
  if(emProcessoDeCalibracao || falhaModuloVisivel) return;
  
  if(telaAtual != 4 && telaAtual != 9 && !(telaAtual > 6)) { // Ajustado para não resetar o scroll na tela do SD
    scrollIndex = 0;
  }
  
  switch (telaAtual) {
    case 0: desenharTelaNotificacoes(); break;
    case 1: desenharHomeScreen(); break;
    case 2: desenharTelaDados(); break;
    case 3: desenharTelaAjustes(); break;
    case 4: desenharTelaDevMode(); break;
    case 5: desenharTelaSobre(); break;
    case 6: desenharTelaPadroes(); break;
    case 7: desenharTelaDev_LoRa(); break;
    case 8: desenharTelaDev_RTC(); break;
    case 9: desenharTelaDev_SD(); break; // A própria função agora decide qual sub-página desenhar
  }
  desenharCabecalho();
}

void desenharBateria() {
    // Lê a tensão já convertida pelo bateria.h
    float tensao = lerTensaoBateria();
    
    // Mapeia de 3.3V (0%) até 4.2V (100%)
    // Multiplicamos por 100 para usar inteiros no map (330 a 420)
    int nivel = map((int)(tensao * 100), 330, 420, 0, 100);
    
    // Trava os limites entre 0 e 100
    if (nivel < 0) nivel = 0;
    if (nivel > 100) nivel = 100;

    // Definições de Tamanho e Posição
    int w = 20; // Largura
    int h = 10; // Altura
    int x = 128 - w - 8; // Canto superior direito
    int y = 4;

    // Desenha o contorno da bateria
    tft.drawRect(x, y, w, h, ST77XX_WHITE);
    // Desenha o terminal positivo (o "biquinho")
    tft.fillRect(x + w, y + 2, 2, h - 4, ST77XX_WHITE);

    // Define a cor baseada na carga
    uint16_t corNivel = ST77XX_GREEN;
    if (nivel < 50) corNivel = ST77XX_YELLOW;
    if (nivel < 20) corNivel = COR_PERIGO; // Vermelho

    // Calcula largura da barra interna (subtrai 4px de borda total)
    int larguraUtil = w - 4;
    int larguraBarra = (larguraUtil * nivel) / 100;

    // Limpa o interior antes de desenhar (para quando a bateria desce)
    tft.fillRect(x + 2, y + 2, larguraUtil, h - 4, COR_FUNDO);
    
    // Desenha a barra de nível
    if (larguraBarra > 0) {
        tft.fillRect(x + 2, y + 2, larguraBarra, h - 4, corNivel);
    }
}



#endif