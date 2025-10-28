#include <WiFi.h>
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <ArduinoJson.h>
#include "time.h"

// --- CONFIGURAÇÕES DE REDE E MQTT ---
const char* ssid = "Gustavo"; // Ou o nome da sua rede Wi-Fi
const char* password = "mqtteste";       // Senha da sua rede
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic_command = "senai/cimatec/robo/comandos/gradin";

// --- CONFIGURAÇÕES DO CALLMEBOT ---
String phoneNumber = "557199687174"; // Ex: 55719...
String apiKey = "3813015";

// --- MAPEAMENTO DE PINOS ---
#define DHTTYPE DHT11
#define DHTpin  25
const int pirPin = 26;
const int LDRpin = 35; 
const int ledRedPin = 14;
const int ledGreenPin = 12;
const int motor_IN1= 19; // controle do motor esquerdo
const int motor_IN2= 18; // controle do motor esquerdo
const int motor_IN3= 4;  // controle do motor direito
const int motor_IN4= 0;  // controle do motor direito


// --- OBJETOS E VARIÁVEIS GLOBAIS ---
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTpin, DHTTYPE);


const char* api_url = "http://10.183.253.145:5000/leituras";


// --- CONFIGURAÇÃO DE HORÁRIO (NOVO) ---
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -3 * 3600; // Fuso Horário Brasil (GMT-3)
const int   daylightOffset_sec = 0;

bool systemEnabled = true; // Robô começa ligado
unsigned long lastMsg = 0; // Para o timer do loop principal

// Protótipos das funções
void callback(char* topic, byte* payload, unsigned int length);
void controlMotors(String command);
void sendWhatsAppAlert();
String obterTimestamp();
void enviarDadosParaAPI(float temp, float umid, int lum, bool pres, int prob);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT...");
    if (client.connect("ESP32_Robo_Explorador")) {
      Serial.println("conectado!");
      // Se inscreve no tópico para receber comandos do controle remoto
      client.subscribe(mqtt_topic_command);
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

// --- FUNÇÃO DE CALLBACK: RECEBE OS COMANDOS DO CONTROLE REMOTO ---
void callback(char* topic, byte* payload, unsigned int length) {
  String command;
  for (int i = 0; i < length; i++) {
    command += (char)payload[i];
  }
  Serial.print("Comando recebido: ");
  Serial.println(command);

  if (command == "DESLIGAR") {
    systemEnabled = false;
    controlMotors("STOP"); // Para os motores imediatamente
  } else {
    // Qualquer outro comando de movimento liga o sistema
    systemEnabled = true;
    controlMotors(command);
  }
}

// --- CONTROLE DOS MOTORES ---
void controlMotors(String command) {
    
        digitalWrite(motor_IN1, LOW);
        digitalWrite(motor_IN2, LOW);
        digitalWrite(motor_IN3, LOW);
        digitalWrite(motor_IN4, LOW);

    if (command == "Frente") {
        digitalWrite(motor_IN1, HIGH);
        digitalWrite(motor_IN2, LOW);
        digitalWrite(motor_IN3, HIGH);
        digitalWrite(motor_IN4, LOW);

    } else if (command == "Tras") {
        digitalWrite(motor_IN1, LOW);
        digitalWrite(motor_IN2, HIGH);
        digitalWrite(motor_IN3, LOW);
        digitalWrite(motor_IN4, HIGH);

    } else if (command == "Esquerda") {
        digitalWrite(motor_IN1, LOW);
        digitalWrite(motor_IN2, LOW);
        digitalWrite(motor_IN3, HIGH);
        digitalWrite(motor_IN4, LOW);
    } else if (command == "Direita") {
        digitalWrite(motor_IN1, HIGH);
        digitalWrite(motor_IN2, LOW);
        digitalWrite(motor_IN3, LOW);
        digitalWrite(motor_IN4, LOW);
    } else if (command == "Parado") {
        digitalWrite(motor_IN1, LOW);
        digitalWrite(motor_IN2, LOW);
        digitalWrite(motor_IN3, LOW);
        digitalWrite(motor_IN4, LOW);
    }
}

// --- FUNÇÃO PARA ENVIAR ALERTA NO WHATSAPP ---
void sendWhatsAppAlert() {
  String message = "Alerta! Alta probabilidade de vida detectada no planeta.";
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpResponseCode = http.GET();
  if (httpResponseCode == 200) {
    Serial.println("Mensagem de alerta enviada com sucesso!");
  } else {
    Serial.print("Erro ao enviar mensagem de alerta! Código: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

// --- FUNÇÃO PARA OBTER TIMESTAMP (NOVO) ---
String obterTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Falha ao obter a hora do servidor NTP");
    return "2025-01-01T00:00:00Z"; // Retorna um valor padrão em caso de falha
  }
  char timeStringBuff[32];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  return String(timeStringBuff);
}


// --- FUNÇÃO PARA ENVIAR DADOS À API ---
void enviarDadosParaAPI(float temp, float umid, int lum, bool pres, int prob) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("API: Sem WiFi, não foi possível enviar dados.");
    return;
  }
  
  HTTPClient http;
  http.begin(api_url);
  http.addHeader("Content-Type", "application/json");


  DynamicJsonDocument doc(256);
  doc["timestamp"] = obterTimestamp();
  doc["temperatura_c"] = temp;
  doc["umidade_pct"] = umid;
  doc["luminosidade"] = lum;
  doc["presenca"] = pres ? 1 : 0; 
  doc["probabilidade_vida"] = prob;

  String jsonPayload;
  serializeJson(doc, jsonPayload);

  int httpCode = http.POST(jsonPayload);

  if (httpCode == 201) { // 201 Created (sucesso, conforme seu código Flask)
    Serial.println("API: Dados enviados com sucesso.");
  } else {
    Serial.printf("API: POST falhou, código: %d - %s\n", httpCode, http.errorToString(httpCode).c_str());
  }

  http.end();
}

void setup() {
  Serial.begin(9600);
  analogReadResolution(12); 
  pinMode(pirPin, INPUT);
  pinMode(ledRedPin, OUTPUT);
  pinMode(ledGreenPin, OUTPUT);
  pinMode(LDRpin, INPUT);
  pinMode(motor_IN1, OUTPUT);
  pinMode(motor_IN2, OUTPUT);
  pinMode(motor_IN3, OUTPUT);
  pinMode(motor_IN4, OUTPUT);

  // Inicializa o sensor DHT
  dht.begin();
  
  // Anexa os servos aos pinos
  // servoL.attach(servoLPin);
  // servoR.attach(servoRPin);
  controlMotors("STOP"); // Garante que os motores comecem parados

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Essencial para o MQTT funcionar

  // Lógica principal que roda a cada 2 segundos (2000 ms)
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    // --- LEITURA DOS SENSORES ---
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    int lightValue = analogRead(LDRpin); // Leitura do LDR
    bool presenceState = digitalRead(pirPin); // Leitura do PIR

    int probability = 0;

    // --- CÁLCULO DA PROBABILIDADE DE VIDA ---
    if (systemEnabled) {
      if (temperature >= 15 && temperature <= 30) {
        probability += 25;
      }
      if (humidity >= 40 && humidity <= 95) {
        probability += 25;
      }
      // O valor de "luz adequada" é um limiar. Ex: > 2000. Ajuste conforme seu sensor e ambiente.
      if (lightValue > 2000) { 
        probability += 20;
      }
      if (presenceState) {
        probability += 30;
      }
    }
    
    // --- LÓGICA DE DECISÃO E ATUADORES ---
    Serial.println("--- Relatório do Robô ---");
    Serial.print("Temperatura: "); Serial.print(temperature); Serial.println(" °C");
    Serial.print("Umidade: "); Serial.print(humidity); Serial.println(" %");
    Serial.print("Intensidade da Luz: "); Serial.println(lightValue);
    Serial.print("Sensor de Presença: "); Serial.println(presenceState ? "Presença detectada" : "Sem presença");
    Serial.print("Probabilidade de Vida: "); Serial.print(probability); Serial.println(" %");

    if (!systemEnabled) {
      digitalWrite(ledGreenPin, LOW);
      digitalWrite(ledRedPin, HIGH);
      Serial.println("Estado do Robô: DESLIGADO");
    } else if (probability > 75) {
      digitalWrite(ledGreenPin, LOW);
      digitalWrite(ledRedPin, HIGH);
      Serial.println("Estado do Robô: ALERTA! Alta probabilidade de vida detectada!");
      sendWhatsAppAlert(); // Envia a mensagem
    } else {
      digitalWrite(ledGreenPin, HIGH);
      digitalWrite(ledRedPin, LOW);
      Serial.println("Estado do Robô: Exploração normal. Nenhum indício relevante detectado.");
    }
    Serial.println("---------------------------\n");
    enviarDadosParaAPI(temperature, humidity, lightValue, presenceState, probability);
  }
  delay(3000);
}