#ifndef SENSOR_SCT013_H
#define SENSOR_SCT013_H

struct DadosSensores;

// --- PINOS ---
#define PINO_DETECCAO_SCT 4       
#define PINO_SENSOR_CORRENTE 35
#define AMOSTRAS_RMS 2000         

// --- CALIBRAÇÃO ---
#define VREF_ESP 3.3
#define ADC_BITS 4095.0

// Valor médio do circuito de polarização sem carga
#define MIDRAIL_VOLTS 1.5492      

#define FATOR_CALIBRACAO 50.0    


// 1. Detecção do Jack com switch: Leitura zero (GND) é desconectado.
const int LIMIAR_CONEXAO_JACK = 20; 

// 2. Corte de Ruído (TRAVA DE 1 AMPERE)
// Qualquer leitura abaixo de 1.0A será enviada como 0.00A.
// Isso elimina a flutuação dos resistores e garante que o LoRa não envie lixo.
const float CORTE_RUIDO = 1.0;      

unsigned long ultimoTempoLeitura_sct = 0;
const unsigned long intervaloLeitura_sct = 300; 

void setup_sct013() {
    pinMode(PINO_DETECCAO_SCT, INPUT_PULLUP);
    analogReadResolution(12); 
    Serial.println("SCT-013: Ini (Corte < 1A)");
}

float calcular_irms_bruto() {
    double somaQuadrados = 0;
    
    for (int i = 0; i < AMOSTRAS_RMS; i++) {
        int leituraADC = analogRead(PINO_SENSOR_CORRENTE);
        
        // Converte ADC para Tensão
        float tensao = (leituraADC * VREF_ESP) / ADC_BITS;
        
        // Subtrai o Zero Calibrado da sua placa
        float tensaoAC = tensao - MIDRAIL_VOLTS;
        
        // Acumula
        somaQuadrados += (tensaoAC * tensaoAC);
    }
    
    float tensaoRMS = sqrt(somaQuadrados / AMOSTRAS_RMS);
    float corrente = tensaoRMS * FATOR_CALIBRACAO;
    
    // --- TRAVA DE 1 AMPERE ---
    // Se o motor não estiver puxando pelo menos 1A, zera tudo.
    if (corrente < CORTE_RUIDO) {
        corrente = 0.0;
    }
    
    return corrente;
}

bool loop_sct013(DadosSensores &dados) {
    // 1. VERIFICAÇÃO DE CONEXÃO
    // Se o pino 4 estiver zerado (GND), o sensor não está lá.
    int leituraJack = analogRead(PINO_DETECCAO_SCT);
    
    if (leituraJack == 0) {
        dados.corrente = 0.0;
        return false; // Erro Amarelo
    }

    // 2. TEMPO
    if (millis() - ultimoTempoLeitura_sct < intervaloLeitura_sct) {
        return true; 
    }
    ultimoTempoLeitura_sct = millis();

    // 3. LEITURA
    dados.corrente = calcular_irms_bruto();
    
    return true; 
}
#endif