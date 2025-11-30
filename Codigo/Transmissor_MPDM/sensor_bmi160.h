#ifndef SENSOR_BMI160_H
#define SENSOR_BMI160_H

#include <Wire.h>
#include <DFRobot_BMI160.h>

const int8_t BMI160_ADDR_CUSTOM = 0x69; 
const float ACCEL_SENSITIVITY = 16384.0;

unsigned long ultimoTempoLeitura_bmi = 0;
const unsigned long intervaloLeitura_bmi = 50; 
DFRobot_BMI160 bmi160;

const int NUM_AMOSTRAS = 20; 
float amostras_ax[NUM_AMOSTRAS] = {0};
float amostras_ay[NUM_AMOSTRAS] = {0};
float amostras_az[NUM_AMOSTRAS] = {0};
int indice_amostra = 0;
float ACCEL_OFFSET_X = 0.0, ACCEL_OFFSET_Y = 0.0, ACCEL_OFFSET_Z = 0.0;
static bool statusBmiOk = true;

void setup_bmi160() {
    // Inicializa a primeira vez
    if (bmi160.I2cInit(BMI160_ADDR_CUSTOM) != BMI160_OK) {
        // Se falhar, não trava o boot, apenas marca erro silencioso
        // O loop vai tentar recuperar depois
    } else {
        // Se iniciou, calibra o offset (média de 50 leituras)
        int16_t d[3];
        float tx = 0, ty = 0, tz = 0;
        for (int i = 0; i < 50; i++) {
            if (bmi160.getAccelData(d) == BMI160_OK) {
                 tx += d[0]; ty += d[1]; tz += d[2];
            }
            delay(5);
        }
        ACCEL_OFFSET_X = (tx / 50.0) / ACCEL_SENSITIVITY * 9.8;
        ACCEL_OFFSET_Y = (ty / 50.0) / ACCEL_SENSITIVITY * 9.8;
        ACCEL_OFFSET_Z = (tz / 50.0) / ACCEL_SENSITIVITY * 9.8; 
    }
}

bool loop_bmi160(DadosSensores &dados) {
    if (millis() - ultimoTempoLeitura_bmi < intervaloLeitura_bmi) {
        return statusBmiOk; 
    }
    ultimoTempoLeitura_bmi = millis();

    // --- MECÂNICA DE AUTO-RECUPERAÇÃO (Heartbeat) ---
    // 1. Testa se o dispositivo responde no barramento I2C
    Wire.beginTransmission(BMI160_ADDR_CUSTOM);
    if (Wire.endTransmission() != 0) {
        // Dispositivo sumiu ou travou o I2C
        // Tenta reinicializar a biblioteca (Soft Reconnect)
        if (bmi160.I2cInit(BMI160_ADDR_CUSTOM) != BMI160_OK) {
            statusBmiOk = false;
            dados.ax = 0; dados.ay = 0; dados.az = 0; // Zera pra não mostrar valor velho
            return false; // Marca erro (Vermelho se estiver em Standby)
        }
    }

    // 2. Tenta ler os dados
    int16_t d[3];
    if (bmi160.getAccelData(d) == BMI160_OK) {
        
        // Se leu, converte
        float fx = (d[0] / ACCEL_SENSITIVITY * 9.8) - ACCEL_OFFSET_X;
        float fy = (d[1] / ACCEL_SENSITIVITY * 9.8) - ACCEL_OFFSET_Y;
        float fz = (d[2] / ACCEL_SENSITIVITY * 9.8) - ACCEL_OFFSET_Z + 9.8;

        // Verifica se está congelado em ZERO exato (muito raro acontecer nos 3 eixos ao mesmo tempo)
        // Se estiver tudo zero absoluto, pode ser falha de leitura da lib
        if (d[0] == 0 && d[1] == 0 && d[2] == 0) {
             // Considera suspeito, mas não falha total ainda, espera a próxima
        }

        // Média Móvel
        amostras_ax[indice_amostra] = fx;
        amostras_ay[indice_amostra] = fy;
        amostras_az[indice_amostra] = fz;
        indice_amostra = (indice_amostra + 1) % NUM_AMOSTRAS;
        
        float sx = 0, sy = 0, sz = 0;
        for (int i = 0; i < NUM_AMOSTRAS; i++) {
            sx += amostras_ax[i]; sy += amostras_ay[i]; sz += amostras_az[i];
        }
        dados.ax = sx / NUM_AMOSTRAS;
        dados.ay = sy / NUM_AMOSTRAS;
        dados.az = sz / NUM_AMOSTRAS; 

        statusBmiOk = true;
        return true;
    }
    
    // Se falhou a leitura da lib
    statusBmiOk = false;
    return false;
}
#endif