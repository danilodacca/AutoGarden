#include <dummy.h>

#include <DHTesp.h> // Inclusão da biblioteca específica para ESP32
#include "Arduino.h"
const int pinoDHT11 = 32; // PINO ANALÓGICO UTILIZADO PELO DHT11
const int umiditypin = 26;

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
int sensorValue;
int temperatura;
int umidade_ar;
int umidade_solo;
int lastValidCondicoes = 3;
int condicoes;
//int lastValidCondicoes;

void setup(){
  Serial.begin(9600); // Inicializa a comunicação serial
  dht.setup(pinoDHT11, DHTesp::DHT11); // Configura o pino e o tipo de sensor
  pinMode(ledPinRed, OUTPUT); // Configurar o pino como saída
  pinMode(ledPinYellow, OUTPUT);
  pinMode(ledPinBlue, OUTPUT);
  pinMode(LHpin1, OUTPUT); // Configurar o pino como saída
  pinMode(LHpin2, OUTPUT); // Configurar o pino como saída
  pinMode(pwmpin, OUTPUT); // Configurar o pino como saída
  pinMode(pwmpin, OUTPUT); // Configurar o pino como saída
  pinMode(umiditypin, INPUT); // Configurar o pino como saída
  
  //ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  //ledcAttachPin(pwmpin, pwmChannel);


  //delay(5000); // Intervalo de 2 segundos antes de iniciar
}

void loop(){
 
  sensorValue = map(analogRead(umiditypin),4095,2000,0,100);
  TempAndHumidity data = dht.getTempAndHumidity(); // Lê as informações do sensor
  Serial.println("------------------------------------------------");
  Serial.print("Umidade: "); // Imprime o texto na serial
  Serial.print(data.humidity); // Imprime na serial o valor de umidade medido
  Serial.print("%"); // Escreve o texto em seguida
  Serial.print(" / Temperatura: "); // Imprime o texto na serial
  Serial.print(data.temperature, 0); // Imprime na serial o valor de temperatura medido e remove a parte decimal
  Serial.println("*C"); // Imprime o texto na serial
  Serial.print("Umidade do solo: ");
  Serial.println(sensorValue);


  temperatura = data.temperature;
  umidade_ar = data.humidity;
  umidade_solo = sensorValue;
  
   // Inicializa com um valor padrão
  condicoes = getCase(umidade_solo, umidade_ar, temperatura);

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
            digitalWrite(ledPinRed, LOW); 
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
            digitalWrite(ledPinRed, LOW); 
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
        case 5:Serial.println("Condição 5: Planta muito fria e seca -> Regar / vermelho.");
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