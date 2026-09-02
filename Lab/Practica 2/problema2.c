#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define TOTAL_PEDIDOS 20

int main() {
    int pipe_banda[2];

    if (pipe(pipe_banda) == -1) {
        perror("Error al crear la tubería");
        exit(EXIT_FAILURE);
    }

    // Crear Estación 1 (Hijo 1)
    pid_t pid1 = fork();

    if (pid1 < 0) {
        perror("Error al crear la Estación 1");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0) {
        // ==================== ESTACIÓN 1 ====================
        close(pipe_banda[1]); // Cierra extremo de escritura

        int pedido = 0;
        int cantidad_pedidos = 0;
        int total_camisas = 0;

        // Lee de la banda transportadora mientras haya datos disponibles
        while (read(pipe_banda[0], &pedido, sizeof(int)) > 0) {
            cantidad_pedidos++;
            total_camisas += pedido;
            usleep(10000); // Pequeña pausa para permitir competencia entre procesos
        }

        printf("\n=========================================\n");
        printf("   REPORTE FINAL: ESTACIÓN 1 (PID: %d)\n", getpid());
        printf("=========================================\n");
        printf("Pedidos procesados: %d\n", cantidad_pedidos);
        printf("Total de camisas despachadas: %d\n", total_camisas);

        close(pipe_banda[0]);
        exit(EXIT_SUCCESS);
    }

    // Crear Estación 2 (Hijo 2)
    pid_t pid2 = fork();

    if (pid2 < 0) {
        perror("Error al crear la Estación 2");
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0) {
        // ==================== ESTACIÓN 2 ====================
        close(pipe_banda[1]); // Cierra extremo de escritura

        int pedido = 0;
        int cantidad_pedidos = 0;
        int total_camisas = 0;

        // Lee de la banda transportadora mientras haya datos disponibles
        while (read(pipe_banda[0], &pedido, sizeof(int)) > 0) {
            cantidad_pedidos++;
            total_camisas += pedido;
            usleep(10000); // Pequeña pausa para permitir competencia entre procesos
        }

        printf("\n=========================================\n");
        printf("   REPORTE FINAL: ESTACIÓN 2 (PID: %d)\n", getpid());
        printf("=========================================\n");
        printf("Pedidos procesados: %d\n", cantidad_pedidos);
        printf("Total de camisas despachadas: %d\n", total_camisas);

        close(pipe_banda[0]);
        exit(EXIT_SUCCESS);
    }

    // ==================== ENCARGADO DE BODEGA (PADRE) ====================
    close(pipe_banda[0]); // Cierra extremo de lectura

    int pedidos[TOTAL_PEDIDOS];
    printf("=========================================\n");
    printf("     BODEGA DE DISTRIBUCIÓN DE CAMISAS   \n");
    printf("=========================================\n");
    printf("Ingrese los %d pedidos (valores entre 1 y 100):\n", TOTAL_PEDIDOS);

    for (int i = 0; i < TOTAL_PEDIDOS; i++) {
        do {
            printf("Pedido #%2d: ", i + 1);
            scanf("%d", &pedidos[i]);
            if (pedidos[i] < 1 || pedidos[i] > 100) {
                printf("  [!] Valor inválido. Debe ser entre 1 y 100.\n");
            }
        } while (pedidos[i] < 1 || pedidos[i] > 100);
    }

    printf("\n[PADRE] Registro completo. Colocando pedidos en la banda transportadora...\n");

    // Escribe todos los valores en la tubería
    for (int i = 0; i < TOTAL_PEDIDOS; i++) {
        write(pipe_banda[1], &pedidos[i], sizeof(int));
    }

    // IMPORTANTE: Cerrar la escritura en el padre para enviar EOF a los hijos
    close(pipe_banda[1]);

    // Esperar a que ambas estaciones terminen sus reportes
    wait(NULL);
    wait(NULL);

    return 0;
}
