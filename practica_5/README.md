# Practica 5

En esta practica se desarolla lo siguiente:
Implementar un módulo de software para utilizar la UART y una MEF para parsear comandos recibidos por UART en modo polling (sin interrupciones ni DMA) usando la HAL de STM32 (STM32F4 + STM32CubeIDE).
En el Punto 1 se crea la capa de acceso a la UART.
En el Punto 2 (progresivo) se implementa un parser de comandos muy sencillo mediante una MEF.
Cabe aclarar que este es el ej1 y practica_4_2 es el ej2.

> El ejercicio 1 no se separo en otro projecto ya que va en conjunto con el 2.

## A tener en cuenta

Si se regenera el codigo del .ioc va a dejar de andar porque inicializa la usart2 (se deberia usar otra pero es más sensillo de debuggear con esta).

En caso de que se regenere, se puede eliminar la linea MX_USART2_UART_Init() en el main.c (algunos warning aparecen por no usar la funcion de inicializacion y la variable del handler).

## Informacion alumno

- Nombre: Potente Guido.
- email: [guidopotente02@gmail.com](mailto:guidopotente02@gmail.com).

# Modo de uso

Durante la inicializacion del dispositivo deberia ver lo siguiente:

```console
USART2|API_UART_BUADRATE|UART_WORDLENGTH_8B|UART_STOPBITS_1|UART_PARITY_NONE|UART_MODE_TX_RX|UART_HWCONTROL_NONE|UART_OVER6
Available commands:
 HELP        -> Show this help message
 LED ON      -> Turn the LED on
 LED OFF     -> Turn the LED off
 LED TOGGLE  -> Toggle the LED state
 STATUS      -> Print the current LED state
 BAUD?       -> Print the current UART baud rate
 BAUD=<n>    -> Set the UART baud rate to <n>

```

Donde la primer linea tiene la configuracion de la uart y el resto es para que pueda ver los comandos disponibles.

A modo de refencia, se dejan listados los comandos.
| Comando | Descripción |
|----------------|-----------------------------------------------------------|
| `HELP` | Muestra la lista de comandos disponibles |
| `LED ON` | Enciende el LED de la placa (LD2) |
| `LED OFF` | Apaga el LED de la placa (LD2) |
| `LED TOGGLE` | Invierte el estado del LED (LD2) |
| `STATUS` | Muestra el estado actual del LED (`on` / `off`) |
| `BAUD?` | Muestra el baud rate actual de la UART |
| `BAUD=<n>` | Cambia el baud rate a `<n>` y reinicializa la UART |

Cabe aclarar que son insensibles a mayusculas y que para enviarlos debe pasar un '\n', '\r' or un '\0'.

Ejemplo:

```console
led on
LED turned on
status
LED is on
baud?
[BAUD] 115200
baud=9600
Setting baudrate to 9600
Baudrate changed
```

> **Importante:** Después de cambiar el baud rate con `BAUD=<n>`, es necesario cambiar también el baud rate en la terminal para poder seguir comunicándose. En minicom se puede hacer en caliente con `Ctrl-A P`.

Si envia algo erroneo o el sistema falla vera alguno de los siguientes errores:
| Error | Causa |
|----------------------|--------------------------------------------------------------------------------|
| `[ERR] OVERFLOW` | La entrada superó el límite del buffer |
| `[ERR] SYNTAX` | El primer carácter es inválido (espacio, tab, `#`, `/` o carácter de control) |
| `[ERR] UNKNOWN CMD` | El comando no coincide con ninguna entrada de la tabla de comandos |
| `[ERR] BAD ARG` | El argumento después de `=` falta, no es un número válido, o es negativo |
| `[ERR] EMPTY CMD` | Se envió un retorno de carro o salto de línea sin ningún carácter previo |

# Testing

El proyecto fue probado con **minicom** como emulador de terminal serie, que puede ser instalado para ubuntu tal que:

```console
sudo apt update
sudo apt install minicom
```

> (Aclaro por las dudas para martin que uso fedora y no me puse a ver si realmente esta en apt minicom).
> Para conectarse se puede hacer:

```console
minicom -D /dev/ttyACM0 -b 115200
```

Si falla es posible que el dispositivo aparezca con otro id, lo que se puede verificar con:

```console
ls /dev/ttyACM*
```

O tambien con:

```console
ls /dev/ttyUSB*
```
Una vez dentro de minicom, para escribir se puede activar el echo con `Ctrl-A E` y cambiar los parametros con  `Ctrl-A P`.

# Compilación
Abrir el proyecto en STM32CubeIDE y compilar con la configuración `Debug`.
Flashear a la placa Nucleo mediante el debugger ST-Link integrado.
