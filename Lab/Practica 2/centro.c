#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define FIFO_NAME "/tmp/fifo_sucursales"
#define BUFFER_SIZE 256

int main() {
    // Crear la FIFO en el sistema de archivos si no existe
    if (mkfifo(FIFO_NAME, 0666) == -1) {
        // Si ya existe, se ignora el error
    }

    printf("=========================================\n");
    printf("      CENTRO DE OPERACIONES (SERVIDOR)   \n");
    printf("=========================================\n");
    printf("[CENTRO] Esperando reportes de sucursales...\n\n");

    // Abrir la FIFO en O_RDWR evita recibir EOF continuo al cerrar cada sucursal
    int fd = open(FIFO_NAME, O_RDWR);
    if (fd == -1) {
        perror("Error al abrir la FIFO");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    double total_acumulado = 0.0;

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(fd, buffer, BUFFER_SIZE - 1);

        if (bytes_read > 0) {
            buffer[strcspn(buffer, "\n")] = 0; // Limpiar salto de línea

            // Evaluación del comando de cierre
            if (strcmp(buffer, "cerrar") == 0) {
                printf("\n[CENTRO] Mensaje 'cerrar' recibido.\n");
                printf("=========================================\n");
                printf("      RESUMEN DE CIERRE DEL DÍA          \n");
                printf("=========================================\n");
                printf("TOTAL ACUMULADO DEL DÍA: Q%.2f\n", total_acumulado);
                printf("=========================================\n");
                break;
            }

            // Captura de hora exacta
            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);
            char hora_str[9];
            strftime(hora_str, sizeof(hora_str), "%H:%M:%S", tm_info);

            double monto = atof(buffer);
            total_acumulado += monto;

            printf("[%s] Reporte recibido: Q%.2f | Total acumulado: Q%.2f\n", hora_str, monto, total_acumulado);
        }
    }

    close(fd);
    unlink(FIFO_NAME); // Elimina el archivo FIFO al finalizar
    return 0;
}
