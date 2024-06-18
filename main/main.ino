#include <DHTesp.h> // Inclusão da biblioteca específica para ESP32
#include "Arduino.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
// Cria uma instância do servidor web
AsyncWebServer server(80);

const int pinoDHT11 = 33; // PINO ANALÓGICO UTILIZADO PELO DHT11
const int umiditypin = 34;

DHTesp dht; // Instância da classe DHTesp

const int ledPinRed = 4; // Pino do LED (você pode usar qualquer pino de saída digital)
const int ledPinBlue = 15;
const int ledPinYellow = 2;
const int LHpin1 = 22;
const int LHpin2 = 23;
const int pwmpin = 21;
const int pwmChannel = 0; // Canal PWM (pode ser 0-15)
const int pwmFreq = 5000; // Frequência PWM em Hz
const int pwmResolution = 8; // Resolução PWM em bits (0-255)
bool ledState = false; // Estado do LED
bool deviceMode = true;
bool BombaState = false;
int temperatura;
String temperaturaStr;
int umidade_ar;
String umidade_arStr;
int umidade_solo;
String umidade_soloStr;
int lastValidCondicoes = 1;
int condicoes;
int umidade_minima = 50;
int temperatura_maxima = 27;
int temperatura_minima = 20;
int temperatura_alarme = 30;
String estadoStr;
SemaphoreHandle_t sensorMutex;

// Função para ler entrada da serial com um prompt
String readSerialInput(String prompt) {
    Serial.println(prompt);
    while (!Serial.available()) {
        // Aguarda o usuário digitar algo
        delay(100);
    }
    return Serial.readStringUntil('\n');
}

// Substitua pelas credenciais da sua rede WiFi
const char* ssid = "AndroidAP9F65";
const char* password = "12345678";

// Tarefa de leitura de sensores
void sensorTask(void *pvParameters) {
    for (;;) {
        xSemaphoreTake(sensorMutex, portMAX_DELAY);
        umidade_solo = map(analogRead(umiditypin), 4095, 1000, 0, 100);
        TempAndHumidity data = dht.getTempAndHumidity();

        temperatura = data.temperature;
        temperaturaStr = String(temperatura);
        umidade_ar = data.humidity;
        umidade_arStr = String(umidade_ar);
        umidade_soloStr = String(umidade_solo);
        xSemaphoreGive(sensorMutex);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Tarefa de controle dos atuadores
void controlTask(void *pvParameters) {
    for (;;) {
        int localCondicoes;
        xSemaphoreTake(sensorMutex, portMAX_DELAY);
        if (deviceMode) {
            localCondicoes = 6;
        } else {
            localCondicoes = getCase(umidade_solo, umidade_ar, temperatura);
        }

        if (localCondicoes == 0) {
            localCondicoes = lastValidCondicoes; // Mantém o último valor válido
        } else {
            lastValidCondicoes = localCondicoes;
        }
        xSemaphoreGive(sensorMutex);

        switch (localCondicoes) {
            case 1:
                Serial.println("Condição 1: Planta fria e seca -> regar / amarelo.");
                estadoStr = "Planta fria e seca -> regar / amarelo.";
                BombaState = true;
                analogWrite(pwmpin , 80);
                digitalWrite(LHpin1 , LOW);
                digitalWrite(LHpin2 , HIGH);
                digitalWrite(ledPinBlue, LOW);
                digitalWrite(ledPinYellow, HIGH);
                digitalWrite(ledPinRed, LOW);
                break;
            case 2:
                Serial.println("Condição 2: Planta quente e seca -> alerta / vermelho.");
                estadoStr = "Planta quente e seca -> alerta / vermelho.";
                BombaState = false;
                digitalWrite(LHpin1 , LOW);
                digitalWrite(LHpin2 , LOW);
                digitalWrite(ledPinBlue, LOW);
                digitalWrite(ledPinRed, HIGH);
                digitalWrite(ledPinYellow, LOW);
                break;
            case 3:
                Serial.println("Condição 3: Planta ok -> azul.");
                estadoStr = "Planta ok -> azul";
                BombaState = false;
                digitalWrite(LHpin1 , LOW);
                digitalWrite(LHpin2 , LOW);
                digitalWrite(ledPinBlue, HIGH);
                digitalWrite(ledPinYellow, LOW);
                digitalWrite(ledPinRed, LOW);
                break;
            case 4:
                Serial.println("Condição 4: Planta muito fria e umida -> alerta para aquecer / amarelo.");
                estadoStr = "Planta muito fria e umida -> aquecer / amarelo.";
                BombaState = false;
                digitalWrite(LHpin1 , LOW);
                digitalWrite(LHpin2 , LOW);
                digitalWrite(ledPinBlue, LOW);
                digitalWrite(ledPinRed, LOW);
                digitalWrite(ledPinYellow, HIGH);
                break;
            case 5:
                Serial.println("Condição 5: Planta muito fria e seca -> Regar / vermelho.");
                estadoStr = "Planta muito fria e seca -> Regar / vermelho.";
                BombaState = true;
                analogWrite(pwmpin , 80);
                digitalWrite(LHpin1 , LOW);
                digitalWrite(LHpin2 , HIGH);
                digitalWrite(ledPinBlue, LOW);
                digitalWrite(ledPinRed, HIGH);
                digitalWrite(ledPinYellow, LOW);
                break;
            case 6:
                Serial.println("Condição 6: Modo automatico desligado");
                break;
            case 7:
                Serial.println("Condição de alarme: Planta muito quente -> alerta vermelho.");
                estadoStr = "Planta muito quente -> alerta vermelho.";
                BombaState = false;
                digitalWrite(LHpin1 , LOW);
                digitalWrite(LHpin2 , LOW);
                digitalWrite(ledPinBlue, LOW);
                digitalWrite(ledPinRed, HIGH);
                digitalWrite(ledPinYellow, HIGH);
                break;
            default:
                Serial.println("Condição não definida.");
                Serial.print("Mantendo funcionamento da condicao anterior ->");
                Serial.println(lastValidCondicoes);
                break;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void setup() {
    Serial.begin(9600);

    // // Solicita SSID e Senha via Serial
    // String ssid = readSerialInput("Digite o SSID: ");
    // String password = readSerialInput("Digite a Senha: ");

    // // Remove espaços extras
    // ssid.trim();
    // password.trim();

    // // Conecta ao WiFi
    // WiFi.begin(ssid.c_str(), password.c_str());
    WiFi.begin(ssid, password);
    Serial.println("Conectando ao WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConectado ao WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP()); // Imprime o endereço IP no monitor serial

    // Define as rotas do servidor
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = "<!DOCTYPE html><html>";
        html += "<head>";
        html += "    <meta charset='UTF-8' />";
        html += "    <meta name='viewport' content='width=device-width, initial-scale=1.0' />";
        html += "    <link rel='stylesheet' href='style.css' />";
        html += "    <title>AutoGarden</title>";
        html += "</head>";
        html += "<style>";
        html += "*{padding: 0; margin: 0; text-decoration: none; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;} .banner2{width: 100%; background-color:rgb(158, 238, 121); background-image: linear-gradient(rgba(0,0,0,0.75),rgba(0,0,0,0.75)),url('../images/pagina_principal.avif'); background-size: cover; background-position: center;} .banner2 h1{color: white; font-size: 50px; text-align: center;} .banner2 h2{color: white; font-size: 35px; text-align: center; margin-bottom: 15px;} .banner2 h3{color: white; font-size: 20px; text-align: center; margin-bottom: 15px;} .button2{width: 200px; padding: 15px 10px; text-align: center; display: block; margin: 20px auto; border-radius: 20px; font-weight: bold; font-size: 20px; border: 2px solid rgb(255, 255, 255); background: transparent; color: white; cursor: pointer; overflow: hidden; transition: all 0.3s ease;} .button2:hover{background: rgb(97, 165, 113); box-shadow: rgb(36, 17, 73); transform: scale(1.1);} .list-container{display: flex; flex-direction: column; align-items: center; justify-content: center; min-height: 100vh; text-align: center;} .button3{width: 150px; padding: 15px 10px; text-align: center; display: block; margin: 20px auto; border-radius: 20px; font-weight: bold; font-size: 20px; border: 2px solid rgb(255, 255, 255); background: transparent; color: white; cursor: pointer; overflow: hidden; transition: all 0.3s ease; margin-bottom: 15px;} .button3:hover{background: rgb(97, 165, 113); box-shadow: rgb(36, 17, 73); transform: scale(1.1);} .formu{color: white; font-size: 20px; text-align: center; margin-bottom: 15px;} .image{width: 20%; display: block; margin-left: auto; margin-right: auto;} html, body{margin: 0; padding: 0; height: 100%; padding-bottom: px;}";
        html += "</style>";
        html += "<body>";
        html += "<div class='banner2'>";
        html += "<h1>AutoGarden</h1>";
        html += "<h3>Umidade do Solo: " + umidade_soloStr + "</h3>";
        html += "<h3>Umidade do Ar: " + umidade_arStr + "</h3>";
        html += "<h3>Temperatura: " + temperaturaStr + "</h3>";
        html += "<h3>Estado: " + estadoStr + "</h3>";
        html += "<h3>Automatic Mode: " + String(deviceMode ? "OFF" : "ON") + "</h3>";
        html += "<button class='button2' onclick=\"toggleMode()\">Trocar Modo</button>";
        if (deviceMode) {
            html += "<h3>Bomba: " + String(BombaState ? "ON" : "OFF") + "</h3>";
            html += "<button class='button2' onclick=\"toggleBOMBA()\">Ativar/Desativar Bomba</button>";
        }
        else {
            html += "<h2>Configuracoes Personalizadas</h2>";
            html += "<form class='formu' action=\"/setParams\" method=\"POST\">";
            html += "Umidade Minima: <input type=\"number\" name=\"umidade_minima\" value=\"" + String(umidade_minima) + "\"><br>";
            html += "Temperatura Minima: <input type=\"number\" name=\"temperatura_minima\" value=\"" + String(temperatura_minima) + "\"><br>";
            html += "Temperatura Maxima: <input type=\"number\" name=\"temperatura_maxima\" value=\"" + String(temperatura_maxima) + "\"><br>";
            html += "Temperatura de Alarme: <input type=\"number\" name=\"temperatura_alarme\" value=\"" + String(temperatura_alarme) + "\"><br>";
            html += "<input class='button3' type=\"submit\" value=\"Enviar\">";
            html += "</form>";
        }
        html += "<script>";
        html += "function toggleMode() {";
        html += "  var xhr = new XMLHttpRequest();";
        html += "  xhr.open('GET', '/toggleMode', true);";
        html += "  xhr.send();";
        html += "  xhr.onload = function() {";
        html += "    if (xhr.status == 200) {";
        html += "      alert(xhr.responseText);";
        html += "      location.reload();"; // Recarrega a página para atualizar o estado do dispositivo
        html += "    }";
        html += "  }";
        html += "}";
        html += "function toggleBOMBA() {";
        html += "  var xhr = new XMLHttpRequest();";
        html += "  xhr.open('GET', '/toggleBOMBA', true);";
        html += "  xhr.send();";
        html += "  xhr.onload = function() {";
        html += "    if (xhr.status == 200) {";
        html += "      alert(xhr.responseText);";
        html += "      location.reload();"; // Recarrega a página para atualizar o estado da bomba
        html += "    }";
        html += "  }";
        html += "}";
        html += "</script>";
        html += "</body></html>";
        request->send(200, "text/html", html);
    });

    // Define a rota para alternar o LED
    server.on("/toggleLED", HTTP_GET, [](AsyncWebServerRequest *request) {
        ledState = !ledState;
        digitalWrite(ledPinRed, ledState ? HIGH : LOW);
        request->send(200, "text/plain", ledState ? "LED is ON" : "LED is OFF");
    });

    // Define a rota para ativar a bomba
    server.on("/toggleBOMBA", HTTP_GET, [](AsyncWebServerRequest *request) {
        BombaState = !BombaState;
        digitalWrite(ledPinBlue, BombaState ? HIGH : LOW);
        analogWrite(pwmpin, 80);
        digitalWrite(LHpin2, BombaState ? HIGH : LOW);
        request->send(200, "text/plain", BombaState ? "Bomba ligada" : "Bomba desligada");
    });

    // Define a rota para alternar o modo de operação
    server.on("/toggleMode", HTTP_GET, [](AsyncWebServerRequest *request) {
        deviceMode = !deviceMode;
        digitalWrite(ledPinBlue, LOW);
        digitalWrite(ledPinRed, LOW);
        digitalWrite(ledPinYellow, LOW);
        if (BombaState){
            digitalWrite(LHpin2 , LOW);
            BombaState = false;

        }
        request->send(200, "text/plain", deviceMode ? "Automatic Mode is OFF" : "Automatic Mode is ON");
    });


    // Define a rota para receber os parâmetros personalizados
    server.on("/setParams", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("umidade_minima", true) && request->hasParam("temperatura_minima", true)
        && request->hasParam("temperatura_maxima", true) && request->hasParam("temperatura_alarme", true)) {
            umidade_minima = request->getParam("umidade_minima", true)->value().toInt();
            temperatura_minima = request->getParam("temperatura_minima", true)->value().toInt();
            temperatura_maxima = request->getParam("temperatura_maxima", true)->value().toInt();
            temperatura_alarme = request->getParam("temperatura_alarme", true)->value().toInt();
            request->send(200, "text/plain", "Parâmetros atualizados");
        } else {
            request->send(400, "text/plain", "Parâmetros inválidos");
        }
    });

    // Inicia o servidor
    server.begin();

    dht.setup(pinoDHT11, DHTesp::DHT11); // Configura o pino e o tipo de sensor
    pinMode(ledPinRed, OUTPUT); // Configurar o pino como saída
    pinMode(ledPinYellow, OUTPUT);
    pinMode(ledPinBlue, OUTPUT);
    pinMode(LHpin1, OUTPUT); // Configurar o pino como saída
    pinMode(LHpin2, OUTPUT); // Configurar o pino como saída
    pinMode(pwmpin, OUTPUT); // Configurar o pino como saída
    pinMode(umiditypin, INPUT); // Configurar o pino como entrada

    // Cria o semáforo para proteger o acesso aos sensores
    sensorMutex = xSemaphoreCreateMutex();

    // Cria as tarefas do FreeRTOS
    xTaskCreate(sensorTask, "Sensor Task", 2048, NULL, 1, NULL);
    xTaskCreate(controlTask, "Control Task", 2048, NULL, 1, NULL);
}

void loop() {
    // Não faz nada. As tarefas são executadas em paralelo.
}

int getCase(int umidade_solo, int umidade_ar, int temperatura) {
    if (umidade_solo < umidade_minima && temperatura < temperatura_maxima) return 1; // Planta fria e seca -> regar / amarelo
    else if (umidade_solo < umidade_minima && temperatura >= temperatura_maxima) return 2; // Planta quente e seca -> alerta / vermelho
    else if (umidade_solo >= umidade_minima && temperatura_minima <= temperatura < temperatura_maxima) return 3; // Planta ok -> azul
    else if (umidade_solo >= umidade_minima && temperatura < temperatura_minima) return 4; // Planta muito fria e umida -> Aquecer
    else if (umidade_solo < umidade_minima && temperatura < temperatura_minima) return 5; // Planta muito fria e seca -> Regar e aquecer
    else if (temperatura > temperatura_alarme) return 7; // Planta MUITO QUENTE -> alarme
    else return 0; // Condição não definida
}
