#include <dummy.h>

#include <DHTesp.h> // Inclusão da biblioteca específica para ESP32
#include "Arduino.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>


// Cria uma instância do servidor web
AsyncWebServer server(80);

const int pinoDHT11 = 32; // PINO ANALÓGICO UTILIZADO PELO DHT11
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
bool deviceMode = false;
bool BombaState = false;
int temperatura;
String temperaturaStr;
int umidade_ar;
String umidade_arStr;
int umidade_solo;
String umidade_soloStr;
int lastValidCondicoes = 1;
int condicoes;
//int lastValidCondicoes;

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
const char* ssid = "5g_ruale";
const char* password = "b1b2b3b4";


void setup(){
  Serial.begin(9600); // Inicializa a comunicação serial

    // Solicita SSID e Senha via Serial
    String ssid = readSerialInput("Digite o SSID: ");
    String password = readSerialInput("Digite a Senha: ");

    // Remove espaços extras
    ssid.trim();
    password.trim();

    //Conecta ao WiFi
    WiFi.begin(ssid.c_str(), password.c_str());
    // WiFi.begin(ssid, password);
    Serial.println("Conectando ao WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConectado ao WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP()); // Imprime o endereço IP no monitor serial

  // Define as rotas do servidor
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html><body>";
    html += "<h1>Controle do ESP32</h1>";
    html += "<h1>ESP32 Monitoramento de Planta</h1>";
    html += "<p>Umidade do Solo: " + umidade_soloStr + "</p>";
    html += "<p>Umidade do Ar: " + umidade_arStr + "</p>";
    html += "<p>Temperatura: " + temperaturaStr + "</p>";



        html += "<p>LED State: " + String(ledState ? "ON" : "OFF") + "</p>";   // LED VERMELHO
        html += "<button onclick=\"toggleLED()\">Toggle LED</button>";         // LED VERMELHO

        html += "<p>Automatic Mode: " + String(deviceMode ? "OFf" : "ON") + "</p>";
        html += "<button onclick=\"toggleMode()\">Toggle Mode</button>";
        if (deviceMode) {
                html += "<p>Bomba: " + String(BombaState ? "ON" : "OFF") + "</p>";
                html += "<button onclick=\"toggleBOMBA()\">Toggle BOMBA</button>";
                //html += "<p>LED State: " + String(ledState ? "ON" : "OFF") + "</p>";
                //html += "<button onclick=\"toggleLED()\">Toggle LED </button>";
            }

    html += "<script>";

    html += "function toggleLED() {";
    html += "  var xhr = new XMLHttpRequest();";
    html += "  xhr.open('GET', '/toggleLED', true);";
    html += "  xhr.send();";
    html += "  xhr.onload = function() {";
    html += "    if (xhr.status == 200) {";
    html += "      alert(xhr.responseText);";
    html += "      location.reload();"; // Recarrega a página para atualizar o estado do LED
    html += "    }";
    html += "  }";
    html += "}";
    
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

  // Define a rota para alternar o LED 1
  server.on("/toggleLED", HTTP_GET, [](AsyncWebServerRequest *request){
    ledState = !ledState;
    digitalWrite(ledPinRed, ledState ? HIGH : LOW);
    request->send(200, "text/plain", ledState ? "LED is ON" : "LED is OFF");
  });

  // Define a rota para ativar a bomba
  server.on("/toggleBOMBA", HTTP_GET, [](AsyncWebServerRequest *request){
    BombaState = !BombaState;
    digitalWrite(ledPinBlue, BombaState ? HIGH : LOW);
    analogWrite(pwmpin , 150);
    digitalWrite(LHpin2 , BombaState ? HIGH : LOW);
            
    request->send(200, "text/plain", BombaState ? "Bomba ligada" : "Bomba desligada");
  });
 

  // Define a rota para alternar o modo de operação
  server.on("/toggleMode", HTTP_GET, [](AsyncWebServerRequest *request){
    deviceMode = !deviceMode;
    request->send(200, "text/plain", deviceMode ? "Automatic Mode is OFF" : "Automatic Mode is ON");
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
  pinMode(pwmpin, OUTPUT); // Configurar o pino como saída
  pinMode(umiditypin, INPUT); // Configurar o pino como entrada
  
  //ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  //ledcAttachPin(pwmpin, pwmChannel);


  //delay(5000); // Intervalo de 2 segundos antes de iniciar
}

void loop(){
 
  umidade_solo = map(analogRead(umiditypin),4095,2000,0,100);
  TempAndHumidity data = dht.getTempAndHumidity(); // Lê as informações do sensor
  Serial.println("------------------------------------------------");
  Serial.print("Umidade: "); // Imprime o texto na serial
  Serial.print(data.humidity); // Imprime na serial o valor de umidade medido
  Serial.print("%"); // Escreve o texto em seguida
  Serial.print(" / Temperatura: "); // Imprime o texto na serial
  Serial.print(data.temperature, 0); // Imprime na serial o valor de temperatura medido e remove a parte decimal
  Serial.println("*C"); // Imprime o texto na serial
  Serial.print("Umidade do solo: ");
  Serial.println(umidade_solo);
  //Serial.println(analogRead(umiditypin));

  temperatura = data.temperature;
  temperaturaStr = String(temperatura);
  umidade_ar = data.humidity;
  umidade_arStr = String(umidade_ar);
  umidade_soloStr = String(umidade_solo);

  if (deviceMode){
    condicoes = 6;
  }
  else{
    condicoes = getCase(umidade_solo, umidade_ar, temperatura);

  }
   // Inicializa com um valor padrão
  
  if (condicoes == 0) {
        condicoes = lastValidCondicoes; // Mantém o último valor válido
    } 

  switch (condicoes) {
        case 1:
            Serial.println("Condição 1: Planta fria e seca -> regar / amarelo.");
            analogWrite(pwmpin , 150);
            digitalWrite(LHpin1 , LOW);
            digitalWrite(LHpin2 , HIGH);
            digitalWrite(ledPinBlue, LOW); // Ligar o LED
            //digitalWrite(ledPinRed, LOW); 
            digitalWrite(ledPinYellow, HIGH);
            lastValidCondicoes = 1;
            delay(1000);
            break;
        case 2:
            Serial.println("Condição 2: Planta quente e seca -> alerta / vermelho.");
            //analogWrite(pwmpin , 220);
            digitalWrite(LHpin1 , LOW);
            digitalWrite(LHpin2 , LOW);
            digitalWrite(ledPinBlue, LOW); // Ligar o LED
            digitalWrite(ledPinRed, HIGH); 
            digitalWrite(ledPinYellow, LOW); 
            lastValidCondicoes = 2;
            // Alerta ao usuário -> wifi
            delay(1000);
            break;
        case 3:
            Serial.println("Condição 3: Planta ok -> azul.");
            //analogWrite(pwmpin , 80);
            digitalWrite(LHpin1 , LOW);
            digitalWrite(LHpin2 , LOW);
            digitalWrite(ledPinBlue, HIGH); // Ligar o LED
            //digitalWrite(ledPinRed, LOW); 
            digitalWrite(ledPinYellow, LOW); 
            lastValidCondicoes = 3;
            delay(1000);
            break;
        case 4:
            Serial.println("Condição 4: Planta muito fria e umida -> alerta para aquecer / amarelo.");
            //analogWrite(pwmpin , 80);
            digitalWrite(LHpin1 , LOW);
            digitalWrite(LHpin2 , LOW);
            digitalWrite(ledPinBlue, LOW); // Ligar o LED
            digitalWrite(ledPinRed, LOW); 
            digitalWrite(ledPinYellow, HIGH); 
            lastValidCondicoes = 4;
            // Alerta ao usuário -> wifi?????
            delay(1000);
            break;
        case 5:
            Serial.println("Condição 5: Planta muito fria e seca -> Regar / vermelho.");
            analogWrite(pwmpin , 50);
            digitalWrite(LHpin1 , LOW);
            digitalWrite(LHpin2 , HIGH);
            digitalWrite(ledPinBlue, LOW); // Ligar o LED
            digitalWrite(ledPinRed, HIGH); 
            digitalWrite(ledPinYellow, LOW); 
            lastValidCondicoes = 5;
            // Alerta ao usuário -> wifi
            delay(1000);
            break;

        case 6:
            Serial.println("Condição 6: Modo automatico desligado");
            // analogWrite(pwmpin , 50);
            // digitalWrite(LHpin1 , LOW);
            // digitalWrite(LHpin2 , HIGH);
            // digitalWrite(ledPinBlue, LOW); // Ligar o LED
            // digitalWrite(ledPinRed, HIGH); 
            // digitalWrite(ledPinYellow, LOW); 
            lastValidCondicoes = 6;
            // Alerta ao usuário -> wifi
            delay(1000);
            break;
        
        default:
            Serial.println("Condição não definida.");
            Serial.print("Mantendo funcionamento da condicao anterior ->");
            Serial.println(lastValidCondicoes);
            break;
    }
    delay(1000);

}

////////////FUNÇÕES

int getCase(int umidade_solo, int umidade_ar, int temperatura) {
    if (umidade_solo < 90 && umidade_ar >= 0 && temperatura < 27) return 1; //Planta fria e seca -> regar / amarelo
    else if (umidade_solo < 90  && umidade_ar >= 0 && temperatura >= 27) return 2;//Planta quente e seca -> alerta / vermelho
    else if (umidade_solo >= 90 && umidade_ar >= 0 && 18 <= temperatura < 27  ) return 3; //Planta ok -> azul
    else if (umidade_solo >= 90 && umidade_ar >= 0 && temperatura < 20) return 4; // Planta muito fria e umida -> Aquecer
    else if (umidade_solo < 90 && umidade_ar >= 0 && temperatura < 20) return 5; // Planta muito fria e seca -> Regar
    else return 0; // Condição não definida
}