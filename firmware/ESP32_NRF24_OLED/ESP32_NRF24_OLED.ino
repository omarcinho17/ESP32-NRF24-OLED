#include "RF24.h"       // Control de los NRF24L01
#include <SPI.h>        // Comunicación SPI
#include <ezButton.h>   // Manejo del interruptor
#include "esp_bt.h"     // Control Bluetooth del ESP32
#include "esp_wifi.h"   // Control WiFi del ESP32
// aqui van las librerias de la oled 
#include <Wire.h>               // Comunicación I2C
#include <Adafruit_GFX.h>       // Gráficos OLED
#include <Adafruit_SSD1306.h>   // Control OLED SSD1306

// Pantalla OLED SSD1306

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_SDA 5
#define OLED_SCL 17

#define OLED_RESET -1

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

bool oledOK = false;


// Punteros para los dos buses SPI
SPIClass *sp = nullptr;
SPIClass *hp = nullptr;

// Configuración de los dos módulos NRF24
RF24 radio(16, 15, 16000000);   // CE 16, CS 15 - HSPI
RF24 radio1(22, 21, 16000000);  // CE 22, CS 21 - VSPI


// Pines utilizados por cada bus SPI
//HSPI=SCK = 14, MISO = 12, MOSI = 13, CS = 15 , CE = 16
//VSPI=SCK = 18, MISO =19, MOSI = 23 ,CS =21 ,CE = 22


// Banderas para controlar la dirección del barrido
unsigned int flag = 0;
unsigned int flagv = 0;

// Canales iniciales
int ch = 45;
int ch1 = 45;


// Interruptor conectado al GPIO 33
ezButton toggleSwitch(33);



void two() {

  // Controla si ch1 aumenta o disminuye
  if (flagv == 0) {

    ch1 += 4;  // Aumenta 4 canales

  } else {

    ch1 -= 4;  // Disminuye 4 canales
  }


  // Controla si ch aumenta o disminuye
  if (flag == 0) {

    ch += 2;   // Aumenta 2 canales

  } else {

    ch -= 2;   // Disminuye 2 canales
  }


  // Cambia la dirección de ch1 al llegar al límite
  if ((ch1 > 79) && (flagv == 0)) {

    flagv = 1;   // Ahora comienza a disminuir

  } else if ((ch1 < 2) && (flagv == 1)) {

    flagv = 0;   // Ahora comienza a aumentar
  }


  // Cambia la dirección de ch al llegar al límite
  if ((ch > 79) && (flag == 0)) {

    flag = 1;   // Ahora comienza a disminuir

  } else if ((ch < 2) && (flag == 1)) {

    flag = 0;   // Ahora comienza a aumentar
  }


  // Cambia el canal de cada radio
  radio.setChannel(ch);
  radio1.setChannel(ch1);


  // Código desactivado para mostrar canales
  /*Serial.print("SP:");
  Serial.println(ch1);
  Serial.print("\tHP:");
  Serial.println(ch);*/
}



void one() {

  // genera canales aleatorios 
  ch1 = random(80);
  ch = random(80);

  // Selecciona canales aleatorios
  radio1.setChannel(ch1);
  radio.setChannel(ch);

  // Pequeña pausa aleatoria
  delayMicroseconds(random(60));


  // Ejemplo de barrido secuencial desactivado
 /*  YOU CAN DO -----SWEEP CHANNEL
  for (int i = 0; i < 79; i++) {
    radio.setChannel(i);
*/

}

void initOLED() {

  // Inicia I2C en los pines elegidos
  Wire.begin(OLED_SDA, OLED_SCL);

  // 0x3C es la dirección más común
  // false evita que la librería vuelva a iniciar I2C en GPIO 21 y 22
  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        0x3C,
        true,
        false)) {

    Serial.println("OLED no encontrada");
    return;
  }

  oledOK = true;

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(18, 16);
  display.println("ESP32 + NRF24");

  display.setCursor(26, 34);
  display.println("Iniciando...");

  display.display();

  delay(1200);
}

void actualizarOLED(int state) {

  // Si no se detectó la pantalla, no hace nada
  if (!oledOK) {
    return;
  }

  // Evita actualizar demasiado rápido
  static unsigned long tiempoAnterior = 0;

  if (millis() - tiempoAnterior < 200) {
    return;
  }

  tiempoAnterior = millis();


  // Limpia la pantalla
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);


  // Título

  display.setCursor(10, 0);
  display.println("ESP32 + 2 NRF24");

  display.drawLine(
    0,
    10,
    127,
    10,
    SSD1306_WHITE
  );

  // Modo

  display.setCursor(0, 17);

  display.print("Modo: ");

  if (state == HIGH) {

    display.println("BARRIDO");

  } else {

    display.println("ALEATORIO");
  }


  // Radio A

  display.setCursor(0, 32);

  display.print("NRF A: CH ");
  display.println(ch);


  // Radio B


  display.setCursor(0, 44);

  display.print("NRF B: CH ");
  display.println(ch1);


  // Switch

  display.setCursor(0, 56);

  display.print("SW: ");

  if (state == HIGH) {

    display.print("HIGH");

  } else {

    display.print("LOW");
  }


  // Envía todo a la pantalla
  display.display();
}



void setup() {

  // Inicia comunicación serial
  Serial.begin(115200);

  // Desactiva Bluetooth y WiFi internos
  esp_bt_controller_deinit();
  esp_wifi_stop();
  esp_wifi_deinit();
  esp_wifi_disconnect();

  // Antirrebote del interruptor
  toggleSwitch.setDebounceTime(50);


  // Inicializa los dos módulos y la OLED
  initHP();
  initSP();
  initOLED();
}



void initSP() {

  // Crea el bus VSPI
  sp = new SPIClass(VSPI);

  // Inicia VSPI
  sp->begin();


  // Comprueba si el radio inició correctamente
  if (radio1.begin(sp)) {

    Serial.println("SP Started !!!");


    // Desactiva confirmación automática
    radio1.setAutoAck(false);

    // Sale del modo recepción
    radio1.stopListening();

    // Desactiva reintentos
    radio1.setRetries(0, 0);

    // Configura potencia máxima
    radio1.setPALevel(RF24_PA_MAX, true);

    // Velocidad de datos de 2 Mbps
    radio1.setDataRate(RF24_2MBPS);

    // Desactiva CRC
    radio1.setCRCLength(RF24_CRC_DISABLED);

    // Muestra información del módulo
    radio1.printPrettyDetails();

    // Inicia portadora continua
    radio1.startConstCarrier(RF24_PA_MAX, ch1);


  } else {

    // Error al iniciar
    Serial.println("SP couldn't start !!!");
  }
}



void initHP() {

  // Crea el bus HSPI
  hp = new SPIClass(HSPI);

  // Inicia HSPI
  hp->begin();


  // Comprueba si el radio inició correctamente
  if (radio.begin(hp)) {

    Serial.println("HP Started !!!");


    // Desactiva confirmación automática
    radio.setAutoAck(false);

    // Sale del modo recepción
    radio.stopListening();

    // Desactiva reintentos
    radio.setRetries(0, 0);

    // Configura potencia máxima
    radio.setPALevel(RF24_PA_MAX, true);

    // Velocidad de datos de 2 Mbps
    radio.setDataRate(RF24_2MBPS);

    // Desactiva CRC
    radio.setCRCLength(RF24_CRC_DISABLED);

    // Muestra información del módulo
    radio.printPrettyDetails();

    // Inicia portadora continua
    radio.startConstCarrier(RF24_PA_MAX, ch);


  } else {

    // Error al iniciar
    Serial.println("HP couldn't start !!!");
  }
}



void loop() {

  // Actualiza el estado del interruptor
  toggleSwitch.loop();


  // Código desactivado para mostrar cambios
  /* if (toggleSwitch.isPressed())
    Serial.println("one");
  if (toggleSwitch.isReleased())
    Serial.println("two");*/


  // Obtiene el estado del interruptor
  int state = toggleSwitch.getState();


  // Si está HIGH ejecuta el modo two()
  if (state == HIGH)

    two();


  // Si está LOW ejecuta el modo one()
  else {

    one();
  }
  //actualiza los datos de la pantalla OLED
  actualizarOLED(state);
}
