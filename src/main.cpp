#include <Arduino.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <ESPAsyncWebServer.h>  // Biblioteca para servidor HTTP assíncrono
#include "SSD1306Wire.h"        // Biblioteca para SSD1306

// ----- Configurações do Wi-Fi -----
const char* ssid = "Vânia gênio / CMFibra"; // Substitua pelo seu SSID
const char* password = "vania061223";     // Substitua pela sua senha

// ----- Configuração do servidor HTTP -----
AsyncWebServer server(80);  // Servidor HTTP na porta 80

// ----- Configuração do Display OLED -----
SSD1306Wire display(0x3c, 5, 4);  // Endereço I2C do display e pinos SDA/SCL

// ----- Definição dos Tempos -----
#define SCAN_TIME 10       // Tempo em segundos para realizar cada escaneamento BLE
#define LOOP_DELAY 5000    // Delay em milissegundos entre um ciclo de scan e outro

// ----- Estrutura para guardar informações de um dispositivo BLE descoberto -----
struct DiscoveredDeviceInfo {
  std::string mac;             // Endereço MAC do dispositivo
  std::string name;            // Nome do dispositivo (se disponível)
  int rssi;                    // Força do sinal recebido (RSSI)
  unsigned long lastSeenTimeMillis; // Timestamp em milissegundos de quando foi visto pela última vez neste ciclo

  // Construtor para facilitar a criação dos objetos
  DiscoveredDeviceInfo(std::string m, std::string n, int r, unsigned long t)
    : mac(m), name(n), rssi(r), lastSeenTimeMillis(t) {}
};

// ----- Variáveis Globais -----
BLEScan* pBLEScan;  // Ponteiro para o objeto que faz o scan BLE
std::vector<DiscoveredDeviceInfo> knownDevices;  // Lista persistente de todos os dispositivos conhecidos
std::vector<DiscoveredDeviceInfo> currentCycleDeviceInfos; // Lista para dispositivos encontrados no ciclo atual
int newDevicesRegisteredThisCycle = 0;  // Contador de novos dispositivos encontrados

// ----- Funções auxiliares -----
bool containsMAC(const std::vector<DiscoveredDeviceInfo>& list, const std::string& mac) {
  for (const auto& item : list) {
    if (item.mac == mac) return true;
  }
  return false;
}

// Função para gerar a página HTML com os dispositivos conhecidos
String generateHTML() {
  String html = "<html><head><style>";
  
  // Definindo o estilo para a tabela e as células
  html += "body { font-family: Arial, sans-serif; background-color: #f4f4f4; color: #333; margin: 0; padding: 0; text-align: center; }";
  html += "h1 { color: #4CAF50; }"; // Título
  html += "h2 { color: #2196F3; }"; // Subtítulo
  html += "table { width: 80%; margin: 20px auto; border-collapse: collapse; }";
  html += "th, td { padding: 10px; text-align: left; border: 1px solid #ddd; }";
  html += "th { background-color: #4CAF50; color: white; }";
  html += "tr:nth-child(even) { background-color: #f2f2f2; }"; // Linhas alternadas
  html += "tr:hover { background-color: #ddd; }"; // Efeito ao passar o mouse sobre as linhas
  
  // Adicionando JavaScript para atualização automática da página
  html += "<script>";
  html += "function refreshPage() {";
  html += "  setTimeout(function() {";
  html += "    location.reload(true);"; // Recarrega a página a cada intervalo de tempo
  html += "  }, 5000);"; // Intervalo de 5 segundos
  html += "}";
  html += "window.onload = refreshPage;";
  html += "</script>";
  
  html += "</style></head><body>";
  
  // Cabeçalho da página
  html += "<h1>Dispositivos BLE Detectados</h1>";

  // Exibe o IP do ESP32 na página
  html += "<h2>IP do ESP32: " + String(WiFi.localIP().toString()) + "</h2>";

  // Tabela para Dispositivos Conhecidos
  html += "<h2>Tabela de Dispositivos Conhecidos</h2>";
  html += "<table><tr><th>MAC</th><th>Nome</th><th>RSSI</th></tr>";

  // Exibe os dispositivos conhecidos
  for (const auto& device : knownDevices) {
    html += "<tr>";
    html += "<td>" + String(device.mac.c_str()) + "</td>";
    html += "<td>" + String(device.name.c_str()) + "</td>";
    html += "<td>" + String(device.rssi) + "</td>";
    html += "</tr>";
  }
  html += "</table>";

  // Tabela para Novos Dispositivos Encontrados
  html += "<h2>Novos Dispositivos Nesse Ciclo</h2>";
  html += "<table><tr><th>MAC</th><th>Nome</th><th>RSSI</th></tr>";

  // Exibe os novos dispositivos encontrados neste ciclo
  for (const auto& device : currentCycleDeviceInfos) {
    if (!containsMAC(knownDevices, device.mac)) {
      html += "<tr>";
      html += "<td>" + String(device.mac.c_str()) + "</td>";
      html += "<td>" + String(device.name.c_str()) + "</td>";
      html += "<td>" + String(device.rssi) + "</td>";
      html += "</tr>";
    }
  }
  html += "</table>";

  html += "</body></html>";
  return html;
}

// Função para atualizar as listas de dispositivos e contadores
// Função para atualizar as listas de dispositivos e contadores
bool addOrUpdateDeviceInCurrentCycleList(BLEAdvertisedDevice& advertisedDevice) {
  std::string mac = advertisedDevice.getAddress().toString(); // Pega o MAC do dispositivo

  // Verifica se o dispositivo já é conhecido
  bool isKnown = containsMAC(knownDevices, mac);

  // Se o dispositivo não for conhecido, adicione à lista de conhecidos
  if (!isKnown) {
    // Adiciona o dispositivo à lista de conhecidos
    knownDevices.push_back(DiscoveredDeviceInfo(
      mac,
      advertisedDevice.haveName() ? advertisedDevice.getName() : "N/A", 
      advertisedDevice.getRSSI(),
      millis()
    ));
    newDevicesRegisteredThisCycle++;  // Novo dispositivo
  }

  // Adiciona o dispositivo à lista do ciclo atual, mesmo que já seja conhecido
  currentCycleDeviceInfos.push_back(DiscoveredDeviceInfo(
    mac,
    advertisedDevice.haveName() ? advertisedDevice.getName() : "N/A", 
    advertisedDevice.getRSSI(),
    millis()
  ));

  return !isKnown;  // Retorna verdadeiro se o dispositivo foi novo, ou falso se foi atualizado
}

// Função para exibir as informações na tela OLED e Serial Monitor
void displayDevicesOnScreen() {
  display.clear();  // Limpa o display antes de desenhar novos dispositivos

  display.setFont(ArialMT_Plain_10);  // Define a fonte para a tela
  display.setTextAlignment(TEXT_ALIGN_LEFT);  // Alinha o texto à esquerda

  int yPosition = 0;  // Posição inicial no eixo Y para desenhar na tela

  // Exibe as contagens de dispositivos no display
  display.drawString(0, yPosition, "New devices: ");
  display.drawString(100, yPosition, String(newDevicesRegisteredThisCycle).c_str());
  yPosition += 10;

  display.drawString(0, yPosition, "Known devices: ");
  display.drawString(100, yPosition, String(knownDevices.size()).c_str()); // Exibe o tamanho da lista de conhecidos
  yPosition += 10;

  display.drawString(0, yPosition, "Devices in cycle: ");
  display.drawString(100, yPosition, String(currentCycleDeviceInfos.size()).c_str());
  yPosition += 10;

  display.display();

  // Exibe as mesmas informações no Serial Monitor
  Serial.println("------ Dispositivos BLE Detectados ------");
  Serial.printf("Novos dispositivos: %d\n", newDevicesRegisteredThisCycle);
  Serial.printf("Dispositivos conhecidos: %d\n", knownDevices.size());
  Serial.printf("Dispositivos no ciclo atual: %d\n", currentCycleDeviceInfos.size());
  Serial.println("-----------------------------------------");

  for (const auto& device : knownDevices) {
    Serial.printf("MAC: %s, Nome: %s, RSSI: %d\n", device.mac.c_str(), device.name.c_str(), device.rssi);
  }
  Serial.println();
}

// ----- Classe para tratar eventos de dispositivos anunciados durante o scan -----
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    addOrUpdateDeviceInCurrentCycleList(advertisedDevice);
  }
};

// Função de configuração inicial do ESP32
void setup() {
  Serial.begin(115200);            // Inicializa comunicação serial para debug
  while (!Serial && millis() < 2000); // Espera a serial ficar pronta (até 2s)
  Serial.println("Inicializando...");

  // Conectar ao Wi-Fi
  WiFi.begin("Vânia gênio / CMFibra", "vania061223");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado ao Wi-Fi");
  
  // Configurar o servidor web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = generateHTML();
    request->send(200, "text/html", html);
  });

  // Inicia o servidor
  server.begin();

  // Inicializar BLE
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

  // Inicializa o display OLED
  display.init();
  display.flipScreenVertically();  // Gira a tela para orientação correta

  Serial.println("Setup concluído.");
}

// ----- Loop principal do programa -----
void loop() {
  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.localIP());

  currentCycleDeviceInfos.clear();  // Limpa os dispositivos detectados no ciclo anterior
  newDevicesRegisteredThisCycle = 0;
  unsigned long cycleStartTime = millis();
  Serial.printf("[%07lu ms] Iniciando novo ciclo de escaneamento BLE...\n", cycleStartTime);

  // Inicia o scan BLE bloqueante por SCAN_TIME segundos
  pBLEScan->start(SCAN_TIME, false);

  unsigned long scanEndTime = millis();
  Serial.printf("[%07lu ms] Escaneamento concluído. Tempo decorrido: %lu ms.\n", scanEndTime, scanEndTime - cycleStartTime);

  // Exibe os dispositivos encontrados e os contadores na tela OLED e Serial Monitor
  displayDevicesOnScreen();

  delay(LOOP_DELAY);  // Espera antes de iniciar novo ciclo
}
