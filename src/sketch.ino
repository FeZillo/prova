#include <WiFi.h>
#include <HTTPClient.h>
#include <Ticker.h>

#define led_verde 41 // Pino utilizado para controle do led verda
#define led_vermelho 40 // Pino utilizado para controle do led vermelho
#define led_amarelo 9 // Pino utilizado para controle do led azul

const int pinoBotao = 18;  // Pino referente ao botão
int estadoBotao = 0;  // Variável para armazenar o estado do botão

const int pinoLdr = 4;  // Pino do sensor LDR
int limite=600; // O limite utilizado

int estado = 0; // 0 -> vermelho; 1 -> amarelo; 2 -> verde

int funcionamento = 0; // 0 -> normal, 1-> noturno

Ticker timerPiscaNoturno;
Ticker timerSemaforo;

void setup() {

  // Configuração inicial dos pinos para controle dos leds como OUTPUTs (saídas) do ESP32
  pinMode(led_amarelo,OUTPUT);
  pinMode(led_verde,OUTPUT);
  pinMode(led_vermelho,OUTPUT);

  // Inicialização das entradas
  pinMode(pinoBotao, INPUT); // Inicializa o botão como input
  pinMode(pinoLdr, INPUT); // Inicializa o LDR como input

  digitalWrite(led_amarelo, LOW);
  digitalWrite(led_verde, LOW);
  digitalWrite(led_vermelho, LOW);

  Serial.begin(9600); // Configuração para debug por interface serial entre ESP e computador com baud rate de 9600

  WiFi.begin("Wokwi-GUEST", ""); // Conexão à rede WiFi aberta com SSID Wokwi-GUEST

  // while (WiFi.status() != WL_CONNECT_FAILED) {
  //   delay(100);
  //   Serial.print(".");
  // }
  // Serial.println("Conectado ao WiFi com sucesso!"); // Considerando que saiu do loop acima, o ESP32 agora está conectado ao WiFi (outra opção é colocar este comando dentro do if abaixo)

  // Verifica estado do botão
  estadoBotao = digitalRead(pinoBotao);
  if (estadoBotao == HIGH) {
    Serial.println("Botão pressionado!");
  } else {
    Serial.println("Botão não pressionado!");
  }

  if(WiFi.status() == WL_CONNECTED){ // Se o ESP32 estiver conectado à Internet
    HTTPClient http;

    String serverPath = "http://www.google.com.br/"; // Endpoint da requisição HTTP

    http.begin(serverPath.c_str());

    int httpResponseCode = http.GET(); // Código do Resultado da Requisição HTTP

    if (httpResponseCode>0) {
      Serial.print("Resposta HTTP: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
      }
    else {
      Serial.print("Código de Erro: ");
      Serial.println(httpResponseCode);
      }
      http.end();
    }

  else {
    Serial.println("WiFi Desconectado");
  }
}

void loop() {
  int ldrstatus=analogRead(pinoLdr);

  if(ldrstatus>=limite){
    Serial.print("Está escuro, acenda a luz");
    Serial.println(ldrstatus);

    if(funcionamento == 0){
      funcionamento = 1;
      timerSemaforo.detach();
      desliga(led_verde);
      desliga(led_vermelho);
      desliga(led_amarelo);
      
      timerPiscaNoturno.attach(1, [](){ alternarAmarelo(); });
    }
    

  }else{
    Serial.print("Está claro, apague a luz");
    Serial.println(ldrstatus);

    if(funcionamento == 1){
      funcionamento = 0;
      timerPiscaNoturno.detach();
      desliga(led_amarelo);
      funcionamentoNormal();

      if(estadoBotao == HIGH){
        timerSemaforo.detach();
        timerSemaforo.once(1, []() { liga(led_verde); });
      }
    }
  }
}

void alternarAmarelo(){
  digitalWrite(led_amarelo, !digitalRead(led_amarelo));
}

void desliga(int pino){
  digitalWrite(pino, LOW);
}

void liga(int pino){
  digitalWrite(pino, HIGH);
}

void funcionamentoNormal() {
  switch (estado) {
    case 0:
      estado = 2;
      liga(led_vermelho);
      desliga(led_amarelo);
      timerSemaforo.once(5, funcionamentoNormal);
    
    case 1: 
      estado = 0;
      liga(led_amarelo);
      desliga(led_verde);
      timerSemaforo.once(2, funcionamentoNormal);
    
    case 2:
      estado = 1;
      liga(led_verde);
      desliga(led_vermelho);
      timerSemaforo.once(3, funcionamentoNormal);
  }
}