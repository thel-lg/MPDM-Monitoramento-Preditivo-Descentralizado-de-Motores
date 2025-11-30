#ifndef ALERTA_H
#define ALERTA_H

#ifndef MAX_RECOVERY_ATTEMPTS
#define MAX_RECOVERY_ATTEMPTS 5
#endif

extern bool falhaLoRa, falhaRTC, falhaSD;
extern bool alertaAmareloAtivo, alertaVermelhoAtivo;
extern bool padroesDefinidos;
extern float corrente, padraoCorrente, vibracaoRMS, padraoVibracao, temperatura, padraoTemperatura;
extern unsigned long ultimoPacoteLoraMs;
extern const long TIMEOUT_LORA;
extern const int BUZZER_PIN; 
extern unsigned long lastSDTryMs; 

extern bool iniciarLoRa(bool reset); 
extern bool iniciarRTC(bool reset);
extern bool iniciarSDCard_Seguro(); 

extern bool falhaHardwareLoRa;
extern int falhaLoRaCount;
extern int falhaRTCCount;
extern int falhaSDCount;
extern bool falhaModuloVisivel;
extern bool falhaModuloAck;

unsigned long timerBipAlerta = 0;
bool estadoBuzzerAlerta = false;

void verificarAlertas() {
    bool alertaSensorAmarelo = false;
    bool alertaSensorVermelho = false;
    
    // --- 1. RECUPERAÇÃO AUTOMÁTICA LORA ---
    // Se o sinal caiu (timeout), tenta dar um Hard Reset no LoRa
    if (falhaLoRa && !falhaHardwareLoRa && falhaLoRaCount < MAX_RECOVERY_ATTEMPTS) {
        falhaLoRaCount++;
        iniciarLoRa(true); // Chama a função com reset=true (Hard Reset)
        if (!falhaLoRa) { 
            falhaLoRaCount = 0; 
        }
    }
    
    // Recuperação RTC
    if (falhaRTC && falhaRTCCount < MAX_RECOVERY_ATTEMPTS) {
        falhaRTCCount++;
        iniciarRTC(true);
        if (!falhaRTC) falhaRTCCount = 0; 
    }
    
    // SD: Ignora (só manual)
    if (falhaSD) { falhaSDCount = 0; }
    
    if (!falhaLoRa && !falhaRTC && !falhaSD) {
        falhaLoRaCount = falhaRTCCount = 0;
    }
        
    bool falhaHardwareCritica = falhaHardwareLoRa || falhaRTC || falhaSD; 

    // --- 2. LÓGICA VALORES ---
    if (padroesDefinidos && padraoVibracao > 0.01) { 
        if (falhaLoRa || 
            (corrente > padraoCorrente * 1.2 && corrente <= padraoCorrente * 1.5) ||
            (temperatura > padraoTemperatura * 1.2 && temperatura <= padraoTemperatura * 1.5) ||
            (vibracaoRMS > padraoVibracao * 1.2 && vibracaoRMS <= padraoVibracao * 1.5)) {
            alertaSensorAmarelo = true;
        }

        if (corrente > padraoCorrente * 1.5 ||
            temperatura > padraoTemperatura * 1.5 ||
            vibracaoRMS > padraoVibracao * 1.5) {
            alertaSensorVermelho = true;
        }
    }

    falhaModuloVisivel = (falhaHardwareCritica) && !falhaModuloAck; 
    alertaVermelhoAtivo = alertaSensorVermelho; 
    alertaAmareloAtivo = alertaSensorAmarelo && !alertaVermelhoAtivo && !falhaModuloVisivel;
    
    // --- 3. SOM (Volume Máximo) ---
    if (alertaVermelhoAtivo) {
        if (millis() - timerBipAlerta > 150) { 
            timerBipAlerta = millis();
            estadoBuzzerAlerta = !estadoBuzzerAlerta;
            if(estadoBuzzerAlerta) analogWrite(BUZZER_PIN, 255); 
            else analogWrite(BUZZER_PIN, 0);
        }
    } else if (alertaAmareloAtivo) {
         if (millis() - timerBipAlerta > 1000) { 
            timerBipAlerta = millis();
            analogWrite(BUZZER_PIN, 255); 
            delay(100); 
            analogWrite(BUZZER_PIN, 0);
        }
    }
}

#endif