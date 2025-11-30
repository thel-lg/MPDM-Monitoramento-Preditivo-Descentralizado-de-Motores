#ifndef SENSOR_DS18B20_H
#define SENSOR_DS18B20_H

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 27
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const int NUM_LEITURAS = 5;
float leituras[NUM_LEITURAS];
int indiceLeitura = 0;
bool bufferCheio = false;
unsigned long ultimoTempoLeitura_ds = 0;
const unsigned long intervaloLeitura_ds = 500;
static bool statusTempOk = true;

void setup_ds18b20() {
    sensors.begin();
    sensors.requestTemperatures();
    for (int i = 0; i < NUM_LEITURAS; i++) leituras[i] = 0;
}

bool loop_ds18b20(DadosSensores &dados) {
    if (millis() - ultimoTempoLeitura_ds < intervaloLeitura_ds) {
        return statusTempOk; 
    }
    ultimoTempoLeitura_ds = millis();

    sensors.requestTemperatures();
    float tempCrua = sensors.getTempCByIndex(0);

    // LÓGICA PLUG AND PLAY
    // Se ler -127 (Desconectado), tenta reiniciar o barramento
    if (tempCrua < -100.0) {
        dados.temperatura = -127.0; 
        sensors.begin(); // <--- ISSO FAZ O PLUG AND PLAY FUNCIONAR
        statusTempOk = false;
        return false; // Erro
    }
    
    statusTempOk = true;
    leituras[indiceLeitura] = tempCrua;
    indiceLeitura++;
    if (indiceLeitura >= NUM_LEITURAS) {
      indiceLeitura = 0;
      bufferCheio = true;
    }

    float soma = 0;
    int count = bufferCheio ? NUM_LEITURAS : indiceLeitura;
    if (count > 0) {
        for (int i = 0; i < count; i++) soma += leituras[i];
        dados.temperatura = soma / count;
    } else {
        dados.temperatura = tempCrua;
    }
    return true;
}
#endif