
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESOS 50

typedef struct {
    char nombre[10];
    int at;          /* tiempo de llegada */
    int bt;          /* rafaga de CPU */
    int ct;          /* tiempo de finalizacion */
    int tat;         /* tiempo de retorno */
    int wt;          /* tiempo de espera */
    int rt;          /* tiempo de respuesta */
    int inicio;      /* instante en que empieza a ejecutar */
    int ejecutado;   /* bandera para SJF: ya fue procesado */
} Proceso;

typedef struct {
    char nombre[10];
    int inicio;
    int fin;
} BloqueGantt;

/* -------- utilidades de entrada con validacion -------- */

int leerEntero(const char *mensaje, int minimo) {
    int valor;
    char buffer[100];
    while (1) {
        printf("%s", mensaje);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("Error de lectura. Intente de nuevo.\n");
            continue;
        }
        if (sscanf(buffer, "%d", &valor) != 1) {
            printf("Entrada invalida. Debe ingresar un numero entero.\n");
            continue;
        }
        if (valor < minimo) {
            printf("El valor debe ser mayor o igual a %d.\n", minimo);
            continue;
        }
        return valor;
    }
}

/* -------- calculo de metricas comunes -------- */

void calcularMetricas(Proceso p[], int n) {
    for (int i = 0; i < n; i++) {
        p[i].tat = p[i].ct - p[i].at;
        p[i].wt  = p[i].tat - p[i].bt;
        p[i].rt  = p[i].inicio - p[i].at;
    }
}

void imprimirResultados(Proceso p[], int n, BloqueGantt gantt[], int numBloques) {
    printf("\n--- Diagrama de Gantt ---\n");
    printf("|");
    for (int i = 0; i < numBloques; i++) {
        printf(" %-4s|", gantt[i].nombre);
    }
    printf("\n0");
    for (int i = 0; i < numBloques; i++) {
        printf("%6d", gantt[i].fin);
    }
    printf("\n");

    printf("\n--- Tabla de metricas ---\n");
    printf("%-6s %-4s %-4s %-4s %-5s %-4s %-4s\n",
           "Proc", "AT", "BT", "CT", "TAT", "WT", "RT");
    int sumaTAT = 0, sumaWT = 0;
    for (int i = 0; i < n; i++) {
        printf("%-6s %-4d %-4d %-4d %-5d %-4d %-4d\n",
               p[i].nombre, p[i].at, p[i].bt, p[i].ct, p[i].tat, p[i].wt, p[i].rt);
        sumaTAT += p[i].tat;
        sumaWT  += p[i].wt;
    }

    printf("\nTiempo de retorno promedio (TAT): %.2f\n", (float)sumaTAT / n);
    printf("Tiempo de espera promedio (WT):    %.2f\n", (float)sumaWT / n);
}

/* -------- FCFS -------- */

void ordenarPorLlegadaFCFS(Proceso p[], int n) {
    /* ordenamiento simple por AT (burbuja) */
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (p[j].at > p[j + 1].at) {
                Proceso temp = p[j];
                p[j] = p[j + 1];
                p[j + 1] = temp;
            }
        }
    }
}

void simularFCFS(Proceso p[], int n) {
    ordenarPorLlegadaFCFS(p, n);

    BloqueGantt gantt[MAX_PROCESOS];
    int tiempoActual = 0;

    for (int i = 0; i < n; i++) {
        if (tiempoActual < p[i].at) {
            tiempoActual = p[i].at; /* CPU esperando a que llegue el proceso */
        }
        p[i].inicio = tiempoActual;
        tiempoActual += p[i].bt;
        p[i].ct = tiempoActual;

        strcpy(gantt[i].nombre, p[i].nombre);
        gantt[i].inicio = p[i].inicio;
        gantt[i].fin = p[i].ct;
    }

    calcularMetricas(p, n);
    imprimirResultados(p, n, gantt, n);
}

/* -------- SJF no expropiativo -------- */

void simularSJF(Proceso p[], int n) {
    BloqueGantt gantt[MAX_PROCESOS];
    int tiempoActual = 0;
    int completados = 0;

    for (int i = 0; i < n; i++) p[i].ejecutado = 0;

    while (completados < n) {
        int idx = -1;
        int menorBT = 999999;

        /* buscar, entre los procesos ya llegados y no ejecutados,
           el de menor rafaga (BT) */
        for (int i = 0; i < n; i++) {
            if (!p[i].ejecutado && p[i].at <= tiempoActual) {
                if (p[i].bt < menorBT) {
                    menorBT = p[i].bt;
                    idx = i;
                }
            }
        }

        if (idx == -1) {
            /* no hay proceso disponible: la CPU esta ociosa,
               avanzamos el reloj hasta la proxima llegada */
            int proximaLlegada = 999999;
            for (int i = 0; i < n; i++) {
                if (!p[i].ejecutado && p[i].at < proximaLlegada) {
                    proximaLlegada = p[i].at;
                }
            }
            tiempoActual = proximaLlegada;
            continue;
        }

        p[idx].inicio = tiempoActual;
        tiempoActual += p[idx].bt;
        p[idx].ct = tiempoActual;
        p[idx].ejecutado = 1;

        strcpy(gantt[completados].nombre, p[idx].nombre);
        gantt[completados].inicio = p[idx].inicio;
        gantt[completados].fin = p[idx].ct;

        completados++;
    }

    calcularMetricas(p, n);
    imprimirResultados(p, n, gantt, n);
}

/* -------- programa principal -------- */

int main() {
    int n, opcion;
    Proceso procesos[MAX_PROCESOS];

    printf("=========================================\n");
    printf("  SIMULADOR DE PLANIFICACION DE PROCESOS \n");
    printf("=========================================\n\n");

    n = leerEntero("Ingrese el numero de procesos a simular: ", 1);
    if (n > MAX_PROCESOS) {
        printf("El numero maximo de procesos es %d. Se ajustara automaticamente.\n", MAX_PROCESOS);
        n = MAX_PROCESOS;
    }

    for (int i = 0; i < n; i++) {
        printf("\n--- Datos del proceso P%d ---\n", i + 1);
        sprintf(procesos[i].nombre, "P%d", i + 1);

        char msgAT[100], msgBT[100];
        sprintf(msgAT, "Tiempo de llegada (AT) de %s: ", procesos[i].nombre);
        sprintf(msgBT, "Rafaga de CPU (BT) de %s: ", procesos[i].nombre);

        procesos[i].at = leerEntero(msgAT, 0);
        procesos[i].bt = leerEntero(msgBT, 1);
    }

    printf("\nSeleccione el algoritmo de planificacion:\n");
    printf("1. FCFS (First Come First Served)\n");
    printf("2. SJF (Shortest Job First)\n");
    opcion = leerEntero("Opcion: ", 1);

    while (opcion != 1 && opcion != 2) {
        printf("Opcion invalida. Debe ser 1 o 2.\n");
        opcion = leerEntero("Opcion: ", 1);
    }

    /* copia para no perder los datos originales al reordenar */
    Proceso copia[MAX_PROCESOS];
    memcpy(copia, procesos, sizeof(Proceso) * n);

    if (opcion == 1) {
        printf("\n>>> Simulando con FCFS <<<\n");
        simularFCFS(copia, n);
    } else {
        printf("\n>>> Simulando con SJF <<<\n");
        simularSJF(copia, n);
    }

    return 0;
}
