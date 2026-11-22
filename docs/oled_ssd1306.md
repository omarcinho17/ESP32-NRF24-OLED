# Pantalla OLED SSD1306

Este proyecto utiliza una pantalla OLED SSD1306 de 0.96 pulgadas con resolución de 128x64 píxeles y comunicación I2C.

## Conexiones

La pantalla OLED utiliza cuatro conexiones:

| OLED | ESP32 |
|---|---|
| GND | GND |
| VCC | 3.3V |
| SDA | GPIO 26 |
| SCL | GPIO 27 |

Se utilizan los GPIO 26 y 27 porque los pines GPIO 21 y GPIO 22 ya están ocupados por uno de los módulos NRF24L01.

## Dirección I2C

La dirección utilizada para la pantalla es:

```text
0x3C
```

## Librerías necesarias

Para utilizar la pantalla OLED se requieren las siguientes librerías:

```text
Adafruit GFX Library
Adafruit SSD1306
Wire
```

`Wire` forma parte del soporte del ESP32 en Arduino IDE.

## Información mostrada

La pantalla OLED se utiliza para mostrar información del funcionamiento del proyecto, como:

- Modo seleccionado mediante el DIP switch.
- Canal actual del NRF24 A.
- Canal actual del NRF24 B.
- Estado general del sistema.

## Inicialización

La pantalla se inicializa mediante la función:

```cpp
initOLED();
```

Esta función configura la comunicación I2C utilizando:

```cpp
SDA = GPIO 26
SCL = GPIO 27
```

y comprueba si la pantalla responde correctamente.

## Actualización de la pantalla

Durante la ejecución del programa se utiliza una función encargada de actualizar la información mostrada en la OLED.

La pantalla no modifica el funcionamiento de los módulos NRF24L01; únicamente funciona como interfaz visual para mostrar el estado del sistema.