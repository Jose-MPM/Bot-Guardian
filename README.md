# Bot-Guardian

## Proyecto de Seguridad con ESP32-CAM y Telegram Bot

Repository where wen find the documentation of the project ESP32CAM-BOT-GUARDIAN, fot the bot we use telegram.
---

Este proyecto es un prototipo de bot-monitoreo que  utiliza un microcontrolador ESP32-CAM conectado con una fotoresistencia LDR y sensor PIR para capturar imágenes cuando se solicite o se detecte un movimiento, realizando la comunicación a través de Telegram, permitimos al usuario solicitar fotos, controlar el flash aunque este programado automaticamente y activar/desactivar el sensor de movimiento.

### Componentes Necesarios

- ESP32-CAM con módulo de energía integrado y cámara OV2640
	- LED para flash integrado
- Sensor de movimiento PIR (Passive Infrared)
- Fotoresistencia (LDR - Light Dependent Resistor)
- Conexión a Internet (Wi-Fi)
- Cable para cargar el codigo a la ESP32
- 6 cables F:F
- Breadboard

#### Configuración Inicial
- Configurar las credenciales de Wi-Fi en las variables ssid y password.
- Obtener un token de bot de Telegram y configurarlo en la variable BOTtoken.
- Configurar el ID de chat en la variable CHAT_ID.

####  Conexiones

- Sensor PIR (Pin 13): Conectar el sensor PIR al pin I/O 13(GPIO13) del ESP32-CAM.
- LDR (Pin 12): Conectar la fotoresistencia al pin I/O 12(GPIO12) del ESP32-CAM.
- Flash LED (Pin 4): Conectar el LED del flash al pin 4 del ESP32-CAM.
- Configuración de la Cámara: La función configInitCamera inicializa la configuración de la cámara, estableciendo los pines y parámetros necesarios.

#### Funcionalidades del Bot de Telegram
- /start: Inicia el bot y muestra información de bienvenida.
- /flash: Cambia el estado del flash LED y muestra el estado actual.
- /photo: Solicita al ESP32-CAM que capture y envíe una foto.
- /motion: Activa/desactiva el sensor de movimiento y muestra el estado actual.

#### Explicación de algunas partes del código:

- Manejo de Mensajes de Telegram
	- La función handleNewMessages procesa los comandos recibidos a través del bot de Telegram, pero primero verifica la autenticidad del usuario y ejecuta las acciones correspondientes dependiendo del mensaje previamente definido, ignoramos cualquier otro mensaje, sin aviso.
- Captura y Envío de Fotos: La función sendPhotoTelegram recibe la foto capturada en ejecución con la cámara y la envía a través del bot de Telegram. También ajusta el flash según la intensidad de luz medida por la fotoresistencia. Sin embargo, para más detalles:
	1. Preparación del Paquete
		1. Definición del Dominio y Variables:
        	- myDomain: Se establece como "api.telegram.org", el dominio de la API de Telegram.
        	- getAll y getBody: Strings para almacenar la respuesta y el cuerpo de la solicitud HTTP.
		2. Conexión al Servidor de Telegram: Se verifica si se logra la conexión al servidor de Telegram a través del cliente TCP.

    	3. Definición de nuestros headers:
        	- head: Cabecera del paquete que incluye información sobre el chat_id y la foto a adjuntar.
        	- tail: Final del paquete, marcando el final de la sección de datos multipart.
    	4. Cálculo de Longitudes:
        	- imageLen: Longitud de los datos de la foto que recibe como parámetro
        	- extraLen: Longitud total de las cabeceras y la cola.
        	- totalLen: Longitud total del paquete, suma de imageLen y extraLen.
    2. Configuración de la Solicitud HTTP
    	    1. Configuración de la Solicitud POST:
        		- Definimos el protocolo y establecemos la solicitud POST con la ruta /bot<token>/sendPhoto.
        		- Se incluyen los headers previamente definidos, como el host, la longitud del contenido y el tipo de contenido multipart/form-data con el límite (boundary):  "NtoryCamBotTutorial".
	3. Envío de Datos de la Imagen: Ocupamos clientTCP.write para enviar los datos al servidor y usamos un bucle for para enviar la imagen en fragmentos de 1024 bytes o el remanente si es menor.
	4. Recepción de la Respuesta
		- Espera de Respuesta y Procesamiento: 
			- Se inicia un temporizador (startTimer) y se espera la respuesta del servidor durante un tiempo límite (waitTime). Durante este tiempo, se lee y procesa la respuesta del servidor.
			- Se acumulan los datos en getBody y verificaMOS si hemos recibido una respuesta completa.
		- Cierre de la Conexión y Retorno de la Respuesta:
			- Se detiene la conexión TCP después de recibir la respuesta completa.
    		- La función retorna la respuesta (getBody) para su posterior procesamiento.
	4. Gestión de Errores
		1. Manejo de Conexiones Fallidas: Si la conexión al servidor no tiene éxito, se establece un mensaje indicando que la conexión ha fallado.
		2. Impresión de Resultados y Liberación de Recursos:
        		- Se imprime en el monitor serial información relevante, como las cabeceras, la cola y el cuerpo de la respuesta.
        		- Con ```esp_camera_fb_return(fb);``` liberamos la memoria(el buffer) empleada por la camara

#### Configuración Inicial del Dispositivo

En la función setup(), se realiza la configuración inicial del dispositivo, abordando los aspectos esenciales para el funcionamiento del sistema.

####  Lógica Principal

- El bucle principal (loop) gestiona la lógica del programa. Verifica si se debe capturar y enviar una foto, detecta movimiento con el sensor PIR y procesa mensajes de Telegram.

    1. Configuración de Pines:
        - Establecemos el modo de los pines para la LDR (fotoresistor), el sensor PIR y el LED de flash.
        - Se utiliza WRITE_PERI_REG para desactivar la protección contra reinicios por bajo voltaje.

    2. Inicialización del Monitor Serial:
        - Se inicia la comunicación con el monitor serial a una velocidad de 115200 baudios.
        - Se introduce un retraso de 500 milisegundos para asegurar la estabilidad en el inicio.

    3. Configuración y Inicialización de la Cámara:
        - Se realiza una llamada a la función configInitCamera() para configurar la cámara ESP32-CAM: Esta función define los pines y ajustes necesarios para el funcionamiento correcto de la cámara.

    4. Conexión a Wi-Fi:
        - Se configura el modo de operación del módulo Wi-Fi como estación (WIFI_STA).
        - Se inicia la conexión a la red Wi-Fi utilizando las credenciales proporcionadas (ssid y password).
        - Se establece un certificado raíz para las conexiones seguras con el servidor de Telegram.

    5. Esperamos hasta la Conexión Exitosa: Se utiliza un bucle while para esperar hasta que el dispositivo esté conectado a la red Wi-Fi.
        - Durante la espera, se imprime un punto en el monitor serial cada 500 milisegundos.

    6. Proporcionamos información sobre la Conexión:
        - Imprimimos en el monitor serial que la conexión fue exitosa, además de la dirección IP asignada al dispositivo por la red Wi-Fi.
        - Se imprime la fuerza de la señal Wi-Fi (Signal Strength).

    7. Se espera un tiempo de 10 segundos antes de enviar el mensaje inicial al bot de Telegram, para que no tengamos una falsa alarma de un movimiento detectado que puede originarme por el sensor PIR al iniciar la ejecución, además imprimimos un mensaje indicando que se debe enviar /start en Telegram para iniciar el bot.

#### Loop

- La función loop() implementa el bucle principal del programa, encargado de manejar la lógica del sistema de monitoreo. 

  	1. Ignorar las Primeras Fotos: Se verifica si es la primera captura (isFirstCapture) para ignorar las primeras dos fotos debido a la calidad inferior mientras la cámara ajusta el balance de blancos.

    2. Preparación de Foto para Envío:
        - Si se solicita enviar una foto (sendPhoto es verdadero), se realiza la lógica para encender o apagar el flash según el valor de la fotoresistencia.
        - Se captura una nueva foto utilizando la función esp_camera_fb_get() y se apaga el flash después de la captura, o imprimimos un mensaje por si hubo un error.
        - Se envía la foto mediante la función sendPhotoTelegram() y se restablecen las variables relacionadas.

    3. Detección de Movimiento:
        - Verificamos y leemos el estado del sensor PIR (MotionDetected).
        - Si se detecta movimiento y no se está enviando una foto, se envía una notificación al bot de Telegram y se activa la bandera para enviar una foto (sendPhoto).

    4. Procesamiento de Mensajes de Telegram:
        - Cada cierto tiempo (definido por botRequestDelay), se verifica si hay nuevos mensajes en el bot de Telegram.
        - Se utiliza la función handleNewMessages() para procesar los mensajes y tomar acciones correspondientes.

- Este proyecto proporciona una solución de seguridad simple y accesible para aquellos que deseen implementar un sistema de monitoreo utilizando el ESP32-CAM y la plataforma de mensajería de Telegram.

## Motivación y Alcance

Este proyecto busca proporcionar una solución de monitoreo eficiente y asequible, centrada en la detección de eventos y la captura de evidencia visual. La combinación de la ESP32-CAM y el sensor PIR ofrece un sistema de vigilancia con notificaciones en tiempo real y control remoto, abordando la creciente importancia de la ciberseguridad.

## Objetivo

El objetivo principal es desarrollar un sistema de monitoreo que, al detectar movimiento, capture una foto y la envíe a través de un bot de Telegram. Esta solución se enfoca en la eficiencia y la accesibilidad, utilizando dispositivos IoT de bajo costo.

##  Introducción
En un contexto de creciente conectividad, la necesidad de soluciones de vigilancia seguras se vuelve crucial. Este proyecto integra la versatilidad de la ESP32-CAM con la capacidad de detección de movimiento del sensor PIR, ofreciendo control remoto y notificaciones en tiempo real para abordar esta necesidad.

##  Resultados
El proyecto ha logrado integrar de manera efectiva la ESP32-CAM y el sensor PIR, con algunas consideraciones sobre falsas alarmas y pequeños retrasos. A pesar de ello, se ha logrado mejorar la accesibilidad y la versatilidad del sistema, convirtiéndolo en una herramienta efectiva de seguridad. Teniendo así una base para poder extender el proyecto a algo más grande.