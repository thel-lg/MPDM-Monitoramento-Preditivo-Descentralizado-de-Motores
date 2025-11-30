#ifndef RELOGIO_H
#define RELOGIO_H

#include <RTClib.h>

// Declaração externa (o objeto real é criado no .ino)
extern RTC_DS3231 rtc;
extern bool falhaRTC;
extern String erroRTC;

// Função para iniciar o relógio
bool iniciarRTC(bool reset) {
    if (!rtc.begin()) {
        falhaRTC = true;
        erroRTC = "Sem I2C";
        return false;
    }
    
    if (rtc.lostPower() || reset) {
        // Se perdeu energia, não ajustamos aqui automaticamente para não sobrepor
        // o ajuste manual que você fará no setup do arquivo principal.
        falhaRTC = false; 
    } else {
        falhaRTC = false;
    }
    
    return true;
}

#endif