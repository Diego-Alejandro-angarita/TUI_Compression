#ifndef STATS_H
#define STATS_H

#include <stdint.h>
#include <time.h>

/* ------------------------------------------------------------
 * StatsReport — estructura compartida entre módulos.
 * Persona 3 la inicializa y acumula; Persona 2 la alimenta;
 * Persona 1 (TUI) la lee para mostrar la pantalla de stats.
 * ------------------------------------------------------------ */

typedef struct {
    /* Volumen */
    uint64_t bytes_original;      /* bytes del contenido sin comprimir  */
    uint64_t bytes_compressed;    /* bytes escritos comprimidos a disco  */

    /* Llamadas de escritura controladas por el programa */
    uint64_t write_calls;         /* veces que el módulo llamó a write() */

    /* Tiempos — se llenan con clock_gettime / getrusage */
    double cpu_user_ms;           /* tiempo CPU modo usuario  (ms)       */
    double cpu_sys_ms;            /* tiempo CPU modo kernel   (ms)       */
    double wall_clock_ms;         /* tiempo real total        (ms)       */

    /* Ratio calculado (0.0 si bytes_original == 0) */
    double compression_ratio;     /* bytes_compressed / bytes_original   */
} StatsReport;

/* Inicializa todos los campos a cero */
void stats_init(StatsReport *report);

/* Calcula compression_ratio a partir de los bytes ya registrados */
void stats_finalize(StatsReport *report);

#endif /* STATS_H */