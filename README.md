[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/3WGUvRN5)
# Laboratorio 3 - Llamadas al Sistema (Avanzado)

En este laboratorio vamos a trabajar con llamadas al sistema relacionadas con el manejo de archivos y creación de procesos.

Las respuestas a las preguntas planteadas en los ejercicios deben ser entregadas en un archivo PDF en el campus virtual.


## Ejercicio 1: Interprete de comandos

El programa `sh.c` es un interprete de comandos (_shell_). Sin embargo, no tiene implementada la funcionalidad de ejecución de programas o de redirección de entrada/salida. Cuando se ejecuta imprime un símbolo de sistema (`$`) y queda a la espera de que el usuario ingrese un comando. Se puede terminar su ejecución con `^C`.

### 1.1: Ejecución de comandos

Implementar la ejecución de comandos. El programa genera una estructura `execcmd` que contiene el comando a ejecutar y sus parámetros (si los hubiera). Para implementar la ejecución de comandos, deben completar el caso `EXEC` en la función `runcmd()`, utilizando la llamada a sistema [`exec()`](http://man7.org/linux/man-pages/man3/exec.3.html). Se debe imprimir un mensaje de error si `exec()` falla, utilizando la función [`perror()`](http://man7.org/linux/man-pages/man3/perror.3.html).

### 1.2: Redirección de E/S

Implementar redirección de E/S mediante los operadores `<` y `>`, de manera que el _shell_ permita ejecutar comandos como:

```console
$ echo "sistemas operativos" > so.txt
$ cat < so.txt
sistemas operativos
$
```

El parser implementado en el _shell_ reconoce estos operadores y genera una estructura `redircmd` con los datos necesarios para implementar la redirección. Deben completar el código necesario en el caso `REDIR` de la función `runcmd()`. Consultar las llamadas al sistema [`open()`](http://man7.org/linux/man-pages/man2/open.2.html) y [`close()`](http://man7.org/linux/man-pages/man2/close.2.html). Imprimir un mensaje de error si alguna de las llamadas al sistema, utilizando [`perror()`](http://man7.org/linux/man-pages/man3/perror.3.html). 

Importante: verificar los permisos con los que se crea el archivo.

## Ejercicio 2: creación y planificación de procesos
El programa `forkprio.c` debe crear un cierto número de procesos hijos, cada uno ejecutando con un valor de prioridad progresivamente más bajo. Luego de un cierto número de segundos se debe envíar una señal de terminación (`SIGTERM`) a todos los procesos hijos. Estos, antes de terminar su ejecución, deben imprimir su PID, prioridad y el número de segundos que ejecutaron.


El siguiente esquema muestra como es la relación con los procesos hijos:
```
      ┌──────────┐                             fork()
      │ forkprio ├───────────┬───────────┬────────────────────┐
      └─────┬────┘           │           │                    │
            │                │           │                    │
            │                │           │                    │
 sleep(secs)│           ┌────▼───┐   ┌───▼────┐          ┌────▼───┐
            .           │ Hijo 1 │   │ Hijo 2 │   ...    │ Hijo n │
            .           └────▲───┘   └───▲────┘          └────▲───┘
            .                │           │                    │
            │                │           │                    │
            │                │           │                    │
            ├────────────────┴───────────┴────────────────────┘
            │                                 kill(SIGTERM)
            │
            ▼
          exit()
```

Requisitos:

- El primer argumento en la linea de comandos debe ser el número de procesos hijos a crear. El segundo parámetro el número de segundos que deben ejecutar. El tercer parámetro indica si se reducen las prioridades (1) o no (0).
- Si el número de segundos es 0 (cero) se debe ejecutar indefinidamente.
- El formato que deben utilizar los procesos hijos al imprimir es `"Child %d (nice %2d):\t%3li\n"` (pid, prioridad, tiempo de ejecución).
- La función `busywork()` en el archivo `forkprio.c` tiene un bucle infinito que repetidamente invoca la función `times()` para consumir tiempo de CPU. Es la función que deben invocar los procesos hijos.

Utilizar las siguientes llamadas al sistema:

* [`setpriority()`](https://man7.org/linux/man-pages/man2/setpriority.2.html): cambia la prioridad de un proceso (también se puede utilizar [`nice()`](https://man7.org/linux/man-pages/man2/nice.2.html)).
* [`getpriority()`](https://man7.org/linux/man-pages/man2/setpriority.2.html): retorna el valor de prioridad un proceso.
* [`getrusage()`](https://man7.org/linux/man-pages/man2/getrusage.2.html): obtiene estadísticas de ejecución de un proceso.
* [`sleep()`](https://man7.org/linux/man-pages/man3/sleep.3.html): suspende la ejecución de un proceso durante el número indicado de segundos.
* [`kill()`](http://man7.org/linux/man-pages/man2/kill.2.html): envía una señal a un proceso.
* [`sigaction()`](http://man7.org/linux/man-pages/man2/sigaction.2.html): permite modificar la acción a ejecutar al recibir una señal.

Para verificar que efectivamente se estan creando los procesos hijos, pueden realizar la siguiente prueba:
```console
$ bin/forkprio 5 0 0 >/dev/null &
$ pstree -cp $(pgrep forkprio | head -1)
$ pkill forkprio
```

Responder:

1. Ejecutar `bin/forkprio 1 0 1 >/dev/null &`. Luego ejecutar el comando `ps -fp $(pgrep forkprio)`. ¿En que estados están los procesos? Explicar.
2. Ejecutar `bin/forkprio 10 30 1 | sort -k 4 -h` y describir el resultado de la ejecución. ¿Por qué el total del tiempo ejecutado por los procesos hijos puede ser mayor que el tiempo que espera el proceso padre?
3. Si el comando anterior se ejecuta indicando que no se cambien las prioridades de los procesos hijos, ¿Cúal es el resultado? Explicar por qué.

---

¡Fin del Laboratorio 3!
