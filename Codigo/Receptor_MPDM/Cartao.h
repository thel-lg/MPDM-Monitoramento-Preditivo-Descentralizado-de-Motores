#ifndef CARTAO_H
#define CARTAO_H

#include <SD.h>

// Declarações externas (variáveis do .ino)
extern RTC_DS3231 rtc;
extern float corrente, temperatura, vibracaoRMS;
// Adicionamos os eixos individuais aqui
extern float eixoX, eixoY, eixoZ; 
extern bool falhaSD;
extern String erroSD;
extern const int SD_CS_PIN; 
extern bool sdCardOperacional; 

void logarErro(String mensagem);
String listarArquivos();

// --- INICIALIZAÇÃO SEGURA ---
bool iniciarSDCard(bool reset) { 
    if (reset) SD.end();

    pinMode(SD_CS_PIN, OUTPUT);
    digitalWrite(SD_CS_PIN, HIGH); 
    delay(10);

    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("SD: Falha de Boot.");
        erroSD = "SEM CARTAO"; 
        falhaSD = true;
        digitalWrite(SD_CS_PIN, HIGH); 
        return false; 
    }
    
    erroSD = "OK";
    falhaSD = false;
    Serial.println("SD: OK");
    return true;
}

bool iniciarSDCard_Seguro() {
    if (iniciarSDCard(true)) {
        sdCardOperacional = true;
        return true;
    } else {
        sdCardOperacional = false;
        return false;
    }
}

// --- GRAVAÇÃO DETALHADA (COLUNAS SEPARADAS) ---
void gravarSD_Seguro() {
    // Proteção Anti-Freeze
    if (!sdCardOperacional) return; 

    // Nome do arquivo por dia (YYYYMMDD.csv)
    DateTime now = rtc.now();
    char nomeArquivo[16];
    sprintf(nomeArquivo, "/%04d%02d%02d.csv", now.year(), now.month(), now.day());

    File dataFile = SD.open(nomeArquivo, FILE_APPEND);
    
    if (dataFile) {
        erroSD = "OK";
        falhaSD = false;

        // 1. DATA (DD/MM/AAAA)
        char dataBuffer[12];
        sprintf(dataBuffer, "%02d/%02d/%04d", now.day(), now.month(), now.year());
        dataFile.print(dataBuffer); 
        dataFile.print(";"); 

        // 2. HORA (HH:MM:SS)
        char horaBuffer[10];
        sprintf(horaBuffer, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
        dataFile.print(horaBuffer); 
        dataFile.print(";");

        // 3. DADOS (Cada um em sua coluna)
        dataFile.print(corrente);       dataFile.print(";");
        dataFile.print(temperatura);    dataFile.print(";");
        dataFile.print(eixoX);          dataFile.print(";");
        dataFile.print(eixoY);          dataFile.print(";");
        dataFile.println(eixoZ);        // println no último para pular linha
        
        dataFile.close();
    } else {
        // Falha de escrita
        sdCardOperacional = false; 
        falhaSD = true;
        erroSD = "REMOVIDO";
        digitalWrite(SD_CS_PIN, HIGH); 
        Serial.println("SD: Erro de escrita (Arquivo do dia).");
    }
}

// Função wrapper para compatibilidade
void cartaoSD() {
    gravarSD_Seguro();
}

void logarErro(String mensagem) {
    if (!sdCardOperacional) return;
    
    File errorFile = SD.open("/ERROS.txt", FILE_APPEND);
    if (errorFile) {
        DateTime now = rtc.now();
        errorFile.print(now.timestamp()); errorFile.print(" - ");
        errorFile.println(mensagem);
        errorFile.close();
    } else {
        sdCardOperacional = false;
        falhaSD = true;
        digitalWrite(SD_CS_PIN, HIGH);
    }
}

String listarArquivos() {
  if (!sdCardOperacional) return "SD Inacessivel";
  
  String lista = "";
  File root = SD.open("/");
  if (!root) {
      falhaSD = true; 
      return "Erro raiz";
  }
  
  root.rewindDirectory();
  File file = root.openNextFile();
  while(file) {
    if (!file.isDirectory()) {
      lista += String("/") + String(file.name()) + "\n";
    }
    file = root.openNextFile();
  }
  root.close();
  
  if (lista == "") return "Vazio";
  return lista;
}

#endif