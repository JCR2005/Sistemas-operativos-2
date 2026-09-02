#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define FIFO_NAME "/tmp/fifo_sucursales"
#define BUFFER_SIZE 256

int main() {
    printf("=========================================\n");
    printf("         SUCURSAL DE PAGOS               \n");
    printf("=========================================\n");

    // Intentar abrir la FIFO creada por centro.c
    int fd = open(FIFO_NAME, O_WRONLY);
    if (fd == -1) {
        perror("[ERROR] No se pudo conectar con el Centro de Operaciones");
        printf("Asegúrese de ejecutar primero ./centro en otra terminal.\n");
        exit(EXIT_FAILURE);
    }

    char mensaje[BUFFER_SIZE];
    printf("Ingrese el monto total reportado (o 'cerrar' para finalizar el día): ");
    fgets(mensaje, BUFFER_SIZE, stdin);
    mensaje[strcspn(mensaje, "\n")] = 0;

    // Enviar información por la tubería con nombre
    write(fd, mensaje, strlen(mensaje) + 1);
    printf("[SUCURSAL] Reporte enviado exitosamente al Centro de Operaciones.\n");

    close(fd);
    return 0;
}
