#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Función para invertir una cadena manualmente (sin librerías externas)
void invertir_cadena(const char *origen, char *destino) {
    int len = 0;
    while (origen[len] != '\0') {
        len++;
    }
    for (int i = 0; i < len; i++) {
        destino[i] = origen[len - 1 - i];
    }
    destino[len] = '\0';
}

int main() {
    // Tubería 1: Padre -> Hijo (para enviar la palabra y la tarjeta)
    // Tubería 2: Hijo -> Padre (para devolver la respuesta del pago)
    int pipe_p2c[2]; 
    int pipe_c2p[2]; 

    if (pipe(pipe_p2c) == -1 || pipe(pipe_c2p) == -1) {
        perror("Error al crear las tuberías");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Error al crear el proceso hijo");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // ==================== PROCESO HIJO ====================
        close(pipe_p2c[1]); // Cierra escritura en P->C
        close(pipe_c2p[0]); // Cierra lectura en C->P

        int realizar_verificacion = 0;
        read(pipe_p2c[0], &realizar_verificacion, sizeof(int));

        // Fase 1: Verificación de canal (opcional)
        if (realizar_verificacion == 1) {
            char buffer_palabra[256];
            int bytes_read = read(pipe_p2c[0], buffer_palabra, sizeof(buffer_palabra));
            if (bytes_read > 0) {
                char invertida[256];
                invertir_cadena(buffer_palabra, invertida);
                printf("\n[HIJO] Canal activo. Confirmación (cadena invertida): %s\n", invertida);
            }
        }

        // Fase 2: Procesamiento de pago
        int numero_tarjeta = 0;
        read(pipe_p2c[0], &numero_tarjeta, sizeof(int));

        char respuesta[20];
        if (numero_tarjeta % 2 == 0) {
            strcpy(respuesta, "PAGO_APROBADO");
        } else {
            strcpy(respuesta, "PAGO_RECHAZADO");
        }

        // Envía la respuesta de vuelta al padre por la segunda tubería
        write(pipe_c2p[1], respuesta, strlen(respuesta) + 1);

        close(pipe_p2c[0]);
        close(pipe_c2p[1]);
        exit(EXIT_SUCCESS);

    } else {
        // ==================== PROCESO PADRE ====================
        close(pipe_p2c[0]); // Cierra lectura en P->C
        close(pipe_c2p[1]); // Cierra escritura en C->P

        int opcion = 0;
        printf("=========================================\n");
        printf("      SISTEMA DE PAGO EN LÍNEA          \n");
        printf("=========================================\n");
        printf("¿Desea realizar verificación de estado? (1 = Sí, 0 = No): ");
        scanf("%d", &opcion);
        getchar(); // Limpiar el salto de línea del búfer de entrada

        write(pipe_p2c[1], &opcion, sizeof(int));

        if (opcion == 1) {
            char palabra[256];
            printf("Ingrese una palabra para probar el canal: ");
            fgets(palabra, sizeof(palabra), stdin);
            palabra[strcspn(palabra, "\n")] = 0; // Eliminar salto de línea

            write(pipe_p2c[1], palabra, strlen(palabra) + 1);
            sleep(1); // Pausa breve para permitir la impresión del hijo
        }

        int tarjeta = 0;
        do {
            printf("\nIngrese su número de tarjeta (entre 1000 y 9999): ");
            scanf("%d", &tarjeta);
        } while (tarjeta < 1000 || tarjeta > 9999);

        // Envía la tarjeta al hijo
        write(pipe_p2c[1], &tarjeta, sizeof(int));

        // Lee la respuesta final enviada por el hijo
        char resultado_pago[20];
        read(pipe_c2p[0], resultado_pago, sizeof(resultado_pago));

        printf("\n[PADRE] Respuesta del sistema de pago: %s\n", resultado_pago);

        close(pipe_p2c[1]);
        close(pipe_c2p[0]);
        wait(NULL); // Esperar a que el hijo termine
    }

    return 0;
}
