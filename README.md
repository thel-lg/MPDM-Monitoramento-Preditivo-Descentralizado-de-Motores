# MPDM - Monitoramento Preditivo de Motores Elétricos

![Status](https://img.shields.io/badge/Status-Concluído-success)
![Platform](https://img.shields.io/badge/Plataforma-ESP32-blue)
![Language](https://img.shields.io/badge/Linguagem-C%2B%2B%2FArduino-orange)

Este repositório contém o firmware e documentação do **MPDM**, um sistema IOT desenvolvido como Trabalho de Conclusão de Curso (TCC) focado na Indústria 4.0 e Manutenção Preditiva.

O sistema realiza a leitura de grandezas físicas críticas de motores elétricos, transmite via LoRa (Long Range) e apresenta os dados em uma Interface Homem-Máquina (IHM) portátil com capacidade de armazenamento histórico.

## 👨‍💻 Equipe de Desenvolvimento

Este projeto foi desenvolvido por:
* **Gustavo Santana Nascimento**
* **Lucas Santos Gama**
* **Nicole da Silva Nunes Pitombeira**
* **Polyanna Silva Borges**

---

## 📐 Arquitetura do Sistema

O projeto é dividido em dois módulos baseados no microcontrolador ESP32:

### 1. Módulo Transmissor (Sensor Node)
Responsável pela aquisição de dados e transmissão sem fio.
* **Processamento:** Leitura de sensores e fusão de dados.
* **Estado:** Máquina de estados (Standby / Transmitting) para economia de energia.
* **Feedback:** LED RGB para indicação visual de status (Erro, Conexão, Leitura).

### 2. Módulo Receptor (Gateway/IHM)
Responsável pelo recebimento, processamento, exibição e armazenamento.
* **Interface:** Display TFT colorido com navegação por menus e gráficos em tempo real.
* **Datalogger:** Gravação de dados em Cartão SD (CSV) com timestamp preciso (RTC).
* **Diagnóstico:** Sistema de alertas visuais e sonoros para falhas de hardware (LoRa, RTC, SD).

## 🛠 Hardware e Componentes

### Sensores e Atuadores
* **Microcontrolador:** ESP32 (DevKit V1)
* **Vibração/Acelerômetro:** BMI160 (Eixos X, Y, Z e RMS)
* **Corrente AC:** SCT-013 (Não invasivo)
* **Temperatura:** DS18B20 (Blindado)
* **Comunicação:** Módulos LoRa SX1278 (433MHz)
* **Armazenamento:** Módulo Micro SD Card SPI
* **Relógio de Tempo Real:** RTC DS3231 (I2C)
* **Display:** TFT 1.8" ST7735 (SPI)

### Pinagem Principal (Definições)

| Componente | Transmissor (GPIO) | Receptor (GPIO) |
| :--- | :--- | :--- |
| **LoRa (SS/RST/DIO0)** | 5 / 17 / 16 | 5 / 17 / 16 |
| **SPI (SCK/MISO/MOSI)** | Padrão VSPI | 18 / 19 / 23 |
| **Display (CS/DC/RST)** | N/A | 15 / 4 / 25 |
| **SD Card (CS)** | N/A | 13 |
| **I2C (SDA/SCL)** | 21 / 22 | 21 / 22 |
| **LEDs Status / Buzzer** | 25, 26, 33 | 14 (Buzzer) |

## 💻 Funcionalidades do Firmware

### Transmissor
* Leitura de sensores com tratamento de erros (`StatusType Enum`).
* Priorização de status via LED RGB (Azul=TX, Vermelho=Erro, Verde=OK).
* Protocolo de envio de estrutura binária (`struct DadosSensores`) para otimizar o payload LoRa.

### Receptor
* **Menu Interativo:** Navegação por botões físicos (Set, Dir, Esq/Analog).
* **Monitoramento de Saúde:** Watchdog via software para LoRa, RTC e SD Card.
* **Algoritmo de Suavização:** Filtro Exponencial (Alpha 0.8) para estabilização de leitura.
* **Calibração:** Modo dedicado para tara de sensores de vibração e corrente.
* **Segurança:** Sistema de arquivos robusto (`iniciarSDCard_Seguro`) para evitar corrupção de dados.

## 📚 Bibliotecas Utilizadas

* `LoRa` (Sandeep Mistry)
* `Adafruit_GFX` & `Adafruit_ST7735`
* `RTClib` (Adafruit)
* `SPI` & `Wire` (Nativas)
* `SD` & `FS` (Nativas ESP32)

## 🚀 Como Executar

1.  Instale o **Arduino IDE** ou **PlatformIO**.
2.  Instale as bibliotecas listadas acima via Gerenciador de Bibliotecas.
3.  Configure a placa como "DOIT ESP32 DEVKIT V1".
4.  Carregue o código da pasta `/Transmissor` no módulo sensor.
5.  Carregue o código da pasta `/Receptor` no módulo IHM.

---

## 🎓 Agradecimentos Especiais

Gostaríamos de agradecer a todos que tornaram este projeto possível:

* **Orientador:** Prof. Eduardo Félix Pereira, pela orientação técnica.
* **Professores:** Jurandir Sá dos Santos, Ronaldo Oliveira, Juliana da Silva e Sérgio Caldeira, pelo auxílio na elaboração técnica.
* **Escola SENAI “Mariano Ferraz”:** Pela infraestrutura e formação técnica.
* **Banca Examinadora:** Pela presença e pelas valiosas considerações.

---
*Projeto desenvolvido para fins acadêmicos - Curso Técnico em Eletroeletrônica - 2025.*
