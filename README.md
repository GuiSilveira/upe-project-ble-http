# Sistema de Detecção BLE com Display OLED e Web Page

Este projeto é um sistema de monitoramento e detecção de dispositivos Bluetooth Low Energy (BLE) utilizando o microcontrolador ESP32. Ele realiza escaneamentos contínuos de dispositivos BLE e exibe os resultados em tempo real em um display OLED local e em uma página web acessível via IP local. O sistema é ideal para monitoramento de dispositivos BLE em ambientes de automação, segurança e rastreamento.

## Lista de Materiais

- **Microcontrolador**: ESP32 Dev Module
  - Arquitetura: 240 MHz, 320KB de RAM, 4MB de Flash
  - Comunicação Wi-Fi e BLE
- **Display OLED**: SSD1306 (I2C)
  - Endereço I2C: 0x3C
  - Pinos I2C: SDA (pino 5), SCL (pino 4)
- **Fonte de Alimentação**:
  - 3.3V via USB ou fonte compatível com o ESP32

## Instruções para Replicar

1. **Configuração do Ambiente**:
   - Instale a plataforma PlatformIO no seu editor de código (Visual Studio Code, por exemplo).
   - Clone ou baixe este repositório no seu ambiente de desenvolvimento.

2. **Conexões de Hardware**:
   - Conecte o **ESP32** ao seu computador via USB.
   - Conecte o **Display OLED SSD1306** ao ESP32 via I2C:
     - **SDA** -> Pino 5
     - **SCL** -> Pino 4

3. **Configuração do Wi-Fi**:
   - No código fonte, altere as variáveis `ssid` e `password` com as credenciais de sua rede Wi-Fi.

4. **Upload do Código**:
   - Utilize o PlatformIO para compilar e carregar o código no ESP32.

5. **Acesso ao Sistema**:
   - Após carregar o código no ESP32, abra o Monitor Serial para ver o IP do dispositivo.
   - No navegador, acesse o IP mostrado para visualizar a página web do sistema.
   - O display OLED exibirá as informações sobre os dispositivos BLE detectados.

6. **Testando o Sistema**:
   - O sistema realizará escaneamentos BLE a cada ciclo de 10 segundos e exibirá os dispositivos conhecidos e novos dispositivos na página web e no display OLED.

## Imagens do Projeto

![Imagem do Projeto 1](https://github.com/GuiSilveira/upe-project-ble-http/raw/main/images/ble-detected-devices-web.png)
![Imagem do Projeto 2](https://github.com/GuiSilveira/upe-project-ble-http/raw/main/images/ble-serial-monitor.png)
![Imagem do Projeto 3](https://github.com/GuiSilveira/upe-project-ble-http/raw/main/images/esp32-display-oled.jpeg)

## Link para a publicação no Hackster.io

[Sistema de Detecção BLE com Display OLED e Web Page no Hackster.io](https://www.hackster.io/guilhermescoutinho/sistema-de-deteccao-ble-com-display-oled-e-web-page-73c29e)

