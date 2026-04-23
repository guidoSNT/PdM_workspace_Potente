# TP Final — Generador de Señales (PdM + PCSE)

Generador de señales basado en el **AD9833 (DDS)**, controlado mediante 6 botones con antirrebote y un potenciómetro (ADC) sobre una **NUCLEO-F446RE**. La configuración actual se muestra por UART usando códigos de escape ANSI.

## A tener en cuenta

Si se regenera el código del `.ioc`, el proyecto va a dejar de andar porque reinicializa USART2 (se debería usar otra pero es más sencillo de depurar con esta).
En caso de que se regenere, eliminar la línea `MX_USART2_UART_Init()` en `main.c` (aparecerán algunos warnings por no usar la función de inicialización y la variable del handler).

## Información del alumno

- **Nombre:** Potente Guido
- **Email:** [guidopotente02@gmail.com](mailto:guidopotente02@gmail.com)

## Funcionalidades

- Control de frecuencia dígito por dígito (botones arriba/abajo/izquierda/derecha).
- Control de fase mediante potenciómetro (ADC, 0–2π mapeado de 0–3.3V).
- Selección de forma de onda cíclica: SINE → TRIANGULAR → SQUARE → SQUARE_DIV.
- Visualización por UART con códigos ANSI (el dígito seleccionado titila con video inverso).
- Botón de carga aplica la configuración actual al AD9833.

## Formato de pantalla

La terminal UART muestra:
12500000.0 Hz ← dígito seleccionado titila
3.142 rads ← fase desde el potenciómetro
SINE ← forma de onda actual

## Botones

| Botón  | Función                                                |
| ------ | ------------------------------------------------------ |
| LOAD   | Aplica la freq/fase/forma de onda actual al AD9833     |
| LEFT   | Mueve cursor a la izquierda (dígito más significativo) |
| RIGHT  | Mueve cursor a la derecha (dígito menos significativo) |
| UP     | Incrementa el dígito seleccionado                      |
| DOWN   | Decrementa el dígito seleccionado                      |
| CHANGE | Cicla entre formas de onda                             |

## Estados de la MEF

| Estado          | Descripción                                                                        |
| --------------- | ---------------------------------------------------------------------------------- |
| Idle            | Estado de reposo. Lee botones y ADC.                                               |
| Procesa entrada | Valida la entrada (ej: rechaza presiones simultáneas de botones).                  |
| Ejecuta         | Procesa la acción del botón (mover cursor, cambiar dígito, cambiar forma de onda). |
| Cambia UART     | Formatea y envía el estado actual a la terminal UART.                              |
| Error           | Envía mensaje de error, vuelve a Idle.                                             |

## Módulos de software

| Archivo(s)               | Descripción                                       |
| ------------------------ | ------------------------------------------------- |
| `API_wg.h/.c`            | MEF del generador de señales y formato ANSI       |
| `API_debounce.h/.c`      | MEF de antirrebote de botones                     |
| `API_debounce_port.h/.c` | Abstracción HAL de GPIO para botones              |
| `AD9833_driver.h/.c`     | Driver del AD9833 (freq, fase, forma de onda)     |
| `AD9833_port.h/.c`       | Abstracción HAL de SPI para el AD9833             |
| `API_adc.h/.c`           | Puerto ADC para lectura de fase por potenciómetro |
| `API_uart.h/.c`          | Comunicación UART                                 |
| `API_delay.h/.c`         | Módulo de delay no bloqueante                     |

## Periféricos

UART, GPIO, SPI (AD9833), ADC (potenciómetro)

## Testing

El proyecto fue probado con tio como emulador de terminal serie:

```console
sudo apt update
sudo apt install tio
```

Conectar:

```console
tio /dev/ttyACM0 -b 115200
```

Fue testeado en Fedora, pero para mantener consistencia con los requisitos del curso se adjunto para debian.
