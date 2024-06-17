#include <dummy.h>

#include <DHTesp.h> // Inclusão da biblioteca específica para ESP32
#include "Arduino.h"
const int pinoDHT11 = 32; // PINO ANALÓGICO UTILIZADO PELO DHT11
const int umiditypin = 26;

DHTesp dht; // Instância da classe DHTesp

const int ledPinRed = 2; // Pino do LED (você pode usar qualquer pino de saída digital)
const int ledPinGreen = 15;
const int ledPinYellow = 4;
const int LHpin1 = 22;
const int LHpin2 = 23;
const int pwmpin = 21;
const int pwmChannel = 0; // Canal PWM (pode ser 0-15)
const int pwmFreq = 5000; // Frequência PWM em Hz
const int pwmResolution = 8; // Resolução PWM em bits (0-255)
int sensorValue;

void setup(){
  Serial.begin(9600); // Inicializa a comunicação serial
  dht.setup(pinoDHT11, DHTesp::DHT11); // Configura o pino e o tipo de sensor
  pinMode(ledPinRed, OUTPUT); // Configurar o pino como saída
  pinMode(ledPinYellow, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);
  pinMode(LHpin1, OUTPUT); // Configurar o pino como saída
  pinMode(LHpin2, OUTPUT); // Configurar o pino como saída
  pinMode(pwmpin, OUTPUT); // Configurar o pino como saída
  pinMode(pwmpin, OUTPUT); // Configurar o pino como saída
  pinMode(umiditypin, INPUT); // Configurar o pino como saída
  
  //ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  //ledcAttachPin(pwmpin, pwmChannel);


  //delay(2000); // Intervalo de 2 segundos antes de iniciar
}

void loop(){
  TempAndHumidity data = dht.getTempAndHumidity(); // Lê as informações do sensor
  Serial.print("Umidade: "); // Imprime o texto na serial
  Serial.print(data.humidity); // Imprime na serial o valor de umidade medido
  Serial.print("%"); // Escreve o texto em seguida
  Serial.print(" / Temperatura: "); // Imprime o texto na serial
  Serial.print(data.temperature, 0); // Imprime na serial o valor de temperatura medido e remove a parte decimal
  Serial.println("*C"); // Imprime o texto na serial
  sensorValue = map(analogRead(umiditypin),4095,2000,0,100);
  Serial.print("Sensor Value: ");
  Serial.println(sensorValue);
  //if (data.humidity>0.85){
  if (sensorValue<50){
    Serial.println("Caso 1");

    analogWrite(pwmpin , 150);

    digitalWrite(LHpin1 , HIGH);
    digitalWrite(LHpin2 , LOW);
    digitalWrite(ledPinGreen, LOW); // Ligar o LED
    digitalWrite(ledPinRed, HIGH); 
    digitalWrite(ledPinYellow, LOW); 
    delay(2000);

    digitalWrite(LHpin1 , LOW);
    digitalWrite(LHpin2 , LOW);
    digitalWrite(ledPinGreen, LOW); // Ligar o LED
    digitalWrite(ledPinRed, LOW); 
    digitalWrite(ledPinYellow, HIGH); 
    delay(2000);
    
    digitalWrite(LHpin1 , LOW);
    digitalWrite(LHpin2 , HIGH);
    digitalWrite(ledPinGreen, HIGH); // Ligar o LED
    digitalWrite(ledPinRed, LOW); 
    digitalWrite(ledPinYellow, LOW); 
    delay(2000);
    
    
    delay(15);
  }
  else{ //if (50<=sensorValue<70){
    Serial.println("Caso 2");
    digitalWrite(LHpin1 , LOW);
    digitalWrite(LHpin2 , LOW);
    analogWrite(pwmpin , 150);
    digitalWrite(ledPinGreen, HIGH); // Ligar o LED
    digitalWrite(ledPinRed, LOW); 
    digitalWrite(ledPinYellow, HIGH); 
    
    delay(15);
  }
  // else{
  //   Serial.println('Caso 3');
  //   digitalWrite(LHpin1 , LOW);
  //   digitalWrite(LHpin2 , HIGH);
  //   analogWrite(pwmpin , 150);
  //   digitalWrite(ledPinGreen, LOW); // Ligar o LED
  //   digitalWrite(ledPinRed, HIGH); 
  //   digitalWrite(ledPinYellow, LOW); 
    
  //   delay(15);
  // }
  delay(2000);
}