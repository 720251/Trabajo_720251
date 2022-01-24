# Trabajo_720251

La finalidad de este proyecto es desarrollar un sistema que permita distinguir entre los diferentes movimientos realizados por el pie de un futbolista al golpear el balón. 
Para ello, se utiliza un sensor inercial y un sistema de adquisición de datos que utilice comunicación inalámbrica para el envío de información en tiempo real. La adquisición de los datos se realiza mediante comunicación WiFi, subiendo la información a un servidor FTP. Para saber la fecha y hora en tiempo real, también mediante comunicación WiFi, se conecta a un servidor NTP para obtenerla.

Para poder generar un algoritmo capaz de distinguir entre diferentes movimientos específicos del futbol durante el choque con el balón es necesario generar un algoritmo previo en el que se recoja la información de los diferentes sensores que se van a integrar para su posterior procesamiento y generación del clasificador.

-----------------RECOGIDA DE DATOS----------------
Este algoritmo previo se encuentra en RecogidaDatos y sigue el principio de funcionamiento de la máquina de estados que aparece en RecogidaDatos.png

La máquina de estados para la experimentación se define de forma que, una vez alimentado, se espera una acción, cuando se detecta un pulso en el pin 14, se pone una variable denominada botón a 1, y se guarda el valor de millis() en ese instante. El tiempo de activación será de 27 segundos. Siempre que la diferencia entre el millis y el botón millis sea menor a este tiempo, habrá unos contadores, encargados de las interrupciones que irán aumentando. Se toman las medidas cada 10 ms y se llama a la función mandaFichero() a los 14 segundos. Cuando la diferencia sea mayor, se pondrán los contadores y la variable botón a cero, se apagará el led se finalizará la conexión FTP.

Como se va a utilizar WiFi, FTP, el sensor MPU9250 que envía la información por I2C y el servidor NTP para la marca temporal, es necesario incluir las librerías correspondientes, además de definir los pines del I2C y el objeto sensor de la librería MPU9250.

Para poder ejecutar correctamente las tomas de datos, el parpadeo del LED y el envío del fichero por FTP se utilizan interrupciones, por lo que es necesario inicializar las variables y funciones que se vayan a designar para este requerimiento.

También se debe definir el WiFi a utilizar, la marca temporal con el servidor NTP y su función, una serie de variables utilizadas para el botón y los millis, así como el protocolo FTP y las variables que se van a utilizar para su manejo.

Una vez definidas todas las variables que se van a utilizar, se procede a definir las funciones.
Se debe definir una función capaz de almacenar los datos. Esta función da el nombre al archivo .csv que se va a enviar, convirtiendo la cadena del nombre del archivo en un array de caracteres. Además, concatena cada conjunto de parámetros tomados en un instante y, de nuevo, los convierte a un array de caracteres.

Estas variables se usan para dar nombre al archivo que se manda por FTP y al contenido que se escribe en ese archivo, tal y como puede verse en la función mandaFichero(). En esta función se inicializa el servidor FTP, abriendo y cerrando la conexión, inicializando el tipo de archivo y el directorio que se va a utilizar.

La función crítica, capaz de realizar la toma de medidas cada 10ms, enviar los datos a los 15seg y hacer parpadear un LED que indica que se está realizando todo ello es la función leeSensor(). Utiliza las interrupciones y el millis() para conseguir realizar todo ello.

Esta función trabaja con interrupciones. Además, como se ha comenzado, cuando se recibe un pulso desde el botón asignado al pin 14, la variable botón se pone a 1 y se guarda el valor millis que había en ese instante.

Siempre que la diferencia entre el millisactual y el valor de millis que había en el instante del pulso del botón sea menor o igual al tiempo de activación, definido en 27seg, y el botón se encuentre a uno se aumentan los contadores.

Cuando el contador totalInterruptCounter llegue a 10, se extraen los parámetros de aceleración y velocidad angular y se llama a la función almacenaDatos() para guardarlos. Además, si el led esta apagando, se enciende. Cada vez que esta variable llegue a 10, se pondrá a cero, de esta forma, se tomarán datos cada 10 ms, tal y como se había propuesto en la máquina de estados.

Cuando el contadorSerial alcance el tiempo de muestreo, fijado en 14seg, Se escribirá el fichero que contiene infoChar por el puerto serie, se llamará a la función mandaFichero() para mandarlo por FTP al servidor y se inicializará a cero el contador.

Para efectuar el parpadeo del led, siempre que este esté en alto y el contadorLed alcance el valor 15, se apagará y se pondrá en contador a cero.

Si la diferencia entre millis actual y el valor de millis que había en el instante del pulso del botón es mayor al tiempo de activación, definido en 27seg, y el botón se encuentra a uno, se inicializan a cero todos los contadores y la variable botón, se apaga el led y se toma actualmillis como millis.

-----------------EXPERIMENTACION----------------

Dado que el objetivo del proyecto es desarrollar un sistema capaz de conseguir distinguir entre diferentes movimientos realizados por el pie de un jugador de fútbol, se decide utilizar un sensor inercial para adquirir la información necesaria.

A partir de una revisión sistemática de la literatura, se plantea utilizar las variables de giroscopio y aceleración, con sus consiguientes parámetros en los ejes X, Y y Z para distinguir entre chutes y pases con el interior y el exterior. Se deberá realizar una exhaustiva toma de datos para estudiar las diferencias de estas variables y sus parámetros en los ejes. De esta forma, estudiando la velocidad de giro de la variable Z, por ejemplo, en la literatura se ha conseguido distinguir entre realizar un disparo frente a un pase o correr sin balón. Además, en la literatura señalan que los pases laterales difieren de los chutes en términos de magnitud y dirección. Para estos pases laterales el giro más importante se realiza en los ejes X e Y, diferenciando entre interior y exterior porque al realizar el giro se obtienen valores contrarios.

Se plantea realizar una experimentación en una pista indoor, con 8 jugadoras de fútbol sala pertenecientes a un equipo que disputa en la liga de 2ª autonómica femenina de Aragón. Además, para la toma de datos, se realizarán 8 pases con el interior y el exterior, y 8 tiros con la puntera y el empeine, todo ello con la pierna derecha.
Todas las tomas de datos deberán ser en tiempo real para poder distinguir cuando se ha realizado el movimiento, por ello el sistema de comunicación deberá ser inalámbrico, sin necesidad de almacenar la información en una micro-SD.

Con los resultados obtenidos, se estudian los valores medios y eficaces de todas las componentes para generar un vector característico. En los resultados obtenidos, se distingue como el impacto con el balón ocasiona aceleraciones muy diferenciadas en los máximos y mínimos, por lo que se va a utilizar esto para analizar una ventana entorno a ese punto. Además, hay que tener en cuenta que, para distinguir entre chute y pase, existen diversas variables estadísticas que nos pueden hacer diferenciar entre uno y otro, como puede ser la media del vector de aceleración, o su valor eficaz. Si bien es cierto que, para distinguir entre tipo de chute o tipo de pase, es primordial reconocer adecuadamente como se realiza el movimiento.

Para un chute con la puntera, la desviación estándar de velocidad angular del giro en el eje X deberá ser menor que si el chute es con el empeine, lo mismo sucede con la componente Z. En cuanto a los pases, la velocidad angular de giro en los ejes X e Y es lo primordial a estudiar, deberá apreciarse un movimiento contrario, esto puede observarse al estudiar el valor eficaz.

Se diseña un árbol de decisión en función de los valores obtenidos para realizar la distinción entre los diferentes movimientos que queremos diferenciar, que podemos ver en ArbolDecision.png

-----------------ALGORITMO CLASIFICACION FUNCIONAL----------------

Una vez diseñado el árbol de decisión, es necesario generar un algoritmo capaz de detectar el movimiento y clasificarlo. Añadir que se requiere una interacción con una app, que debe dar la orden de comienzo y paro en cuanto a detección de movimientos, además de recibir la información sobre el tipo de movimiento efectuado siempre que entre dentro de las especificaciones requeridas.

Se diseña una máquina de estados para ayudarnos con el desarrollo del algoritmo que podemos ver en AlgoritmoClasificacionFuncional.png

Para la interacción con la app se va a utilizar el protocolo MQTT, suscribiéndonos para recibir en el dispositivo la acción de start/stop, y publicando desde el dispositivo a la app el tipo de movimiento realizado.

Como en el algoritmo de toma de datos, se requiere la utilización de una serie de librerías para obtener fecha y hora por servidor NTP, WiFi, manejo del sensor MMPU8250 y el envío de sus datos por I2C, y MQTT, además de definir los pines del I2C y el objeto sensor de la librería MPU9250.

Para poder ejecutar correctamente las tomas de datos se utilizan interrupciones, por lo que es necesario inicializar las variables y funciones que se vayan a designar para este requerimiento

También se debe definir el WiFi a utilizar, el cliente MQTT junto con el broker al que nos vamos a conectar, con un nombre y puerto específico, la marca temporal con el servidor NTP y su función, y una serie de variables que se van a utilizar en el algoritmo.

Una vez definidas todas las variables que se van a utilizar, se procede a definir las funciones.

La función muestreo() actualiza los valores de aceleración y velocidad angular y los va almacenando en función del aumento de la variable “i” en unos arrays determinados, cuando i llega a 300, la inicializa a cero.

La función ventana(), como su propio nombre indica, genera una ventana en la que, si los valores de la aceleración en el eje Z en un instante dando son 4 o -4 y aun no se ha iniciado la ventana, guarda el valor “i” del instante en la variable “j”, toma el valor de millis en ese instante y pone la variable ventabaIniciada a uno. Si el valor de j es mayor o igual a 75, guarda los valores iniciales y finales tal y como aparece en el código. Si el valor de j es menor a 75, se debe tomar el valor inicial como se indica.

Esto es así porque se han creado unos arrays de 300 posiciones, estas posiciones se rellenan cada 10ms y el estudio de la ventana se realiza en un intervalo de 1500 ms, es decir, en 150 posiciones. Se debe estudiar en que posición del array se ha efectuado el choque con el balón y estudiar las 75 posiciones anteriores y posteriores. Cuando la posición es igual o mayor a 75, no existe problema de detectar cuando comienza y finaliza el intervalo a estudiar. Sin embargo, cuando la posiciones mejor a 75, algunos de los valores se encuentran en la ventana rellenada en los 3000 ms anteriores, por lo que se deben coger los últimos valores del array antes de que vuelvan a sobreescribirse.

Una vez sabemos cual es el instante inicial, utilizamos una variable auxiliar para que se rellene en ella la posición en la que nos encontramos por medio de un for. Esto lo podemos encontrar en la función analisis(). Se utiliza para poder hacer un algoritmo específico y obtener la media de cada uno de los ejes, tanto de aceleración como de velocidad angular, así como los valores eficaces, y las normas de ambos. Se vuelve a realizar lo mismo con otra variable auxiliar para obtener la desviación estándar y, finalmente, se inicializa todos los acumuladores a cero para que no entren en un bucle infinito de acumulación.

Finalmente encontramos la función de clasificacion(). En ella aplicamos el árbol de decisión diseñado anteriormente para distinguir entre los diferentes movimientos que queremos detectar, y publicamos por MQTT el mensaje definido en la variable parameter, un string compuesto por el movimiento efectuado, la fecha y la hora.

Es importante mencionar que, para poder leer la suscripción al broker de mqtt es necesario utilizar dos funciones, la de conexión, que solo se ejecuta una vez al conectarse al broker y la de cómo utilizar la información que nos llega por mqtt.

En el setup, como en casos anteriores, debemos inicializar el I2C para el envío de la información del sensor, el acelerómetro y el giróscopo, las interrupciones que se van a utilizar, en este caso, en el bucle principal, el WiFi, la configuración del servidor NTP, y el cliente de MQTT.

En el bucle principal, llamamos a las diferentes funciones siempre que el cliente MQTT esté conectado.

Además, utilizamos las interrupciones para muestrear, analizar la ventana y clasificar. De esta forma, siempre que la variable botón esté activa, se aumentaran unos contadores por cada interrupción que se ejecute y se llamará a la función ventana().

Cuando el contador de muestreo sea igual al periodo de muestreo, se llama a la función muestreo() para que se ejecute y se pone a cero dicho contador. 

Señalar que se llama a la función análisis siempre que la diferencia entre actualmillis y ventanamillis (obtenida cuando se ha detectado un choque con el balón) sea menor o igual al periodo de la ventana. Además, cuando el contador clasificación sea igual al periodo de clasificación, se llamará a la función clasificacion() y se pondrá a cero dicho contador.

Cuando la diferencia entre los millis supere el periodo de la ventana, actualmillis será igual a millis y la ventana iniciada se podrá a cero, desactivándose.

Para el diseño de la app, se ha descargado la aplicación MQTT Dash (IoT, Smart Home) y se ha configurado de forma que, se da el nombre que queremos, utilizando un broker público para conectarse a el y utilizarlo para enviar la información, el mismo que en el algoritmo. Se crea un texto con el topic “Sensor/mensaje” tal y como se nombra en la línea 229 del algoritmo, para leer lo que se publica en ese topic. Y se crea un botón con el topic “Sensor/boton” tal y como se nombra en la línea 274 del algoritmo, para enviar a la suscripción lo que hay en ese topic.

Finalmente, se verifica y se comprueba que el algoritmo funciona correctamente, recibiendo los mensajes de pase con el interior o pase con el exterior si entra dentro de las especificaciones, o pase si no distingue entre que tipo de pase es, y de chute con el empeine o chute con la puntera si entra dentro de especificaciones, o chute si no distingue entre el tipo de chute.

