#ifndef STATS_H
#define STATS_H

#include <stdint.h>

/* ------------------------------------------------------------
 * StatsReport — estructura compartida entre módulos.
 * Persona 3 la inicializa y acumula; Persona 2 la alimenta;
 * Persona 1 (TUI) la lee para mostrar la pantalla de stats.
 * ------------------------------------------------------------ */

typedef struct {
    /* Volumen */
    uint64_t bytes_original;      /* bytes del contenido sin comprimir  */
    uint64_t bytes_compressed;    /* bytes escritos comprimidos a disco  */
    uint64_t bytes_written;       /* bytes escritos a disco por el flujo */

    /* Llamadas de escritura controladas por el programa */
    uint64_t write_calls;         /* veces que el módulo llamó a write() */

    /* Tiempos — se llenan con clock_gettime / getrusage */
    double cpu_user_ms;           /* tiempo CPU modo usuario  (ms)       */
    double cpu_sys_ms;            /* tiempo CPU modo kernel   (ms)       */
    double wall_clock_ms;         /* tiempo real total        (ms)       */

    /* Ratio calculado (0.0 si bytes_original == 0) */
    double compression_ratio;     /* bytes_compressed / bytes_original   */
} StatsReport;

typedef struct {
    double wall_start_ms;
    double cpu_user_start_ms;
    double cpu_sys_start_ms;
} StatsTimer;

/* Inicializa todos los campos a cero */
void stats_init(StatsReport *report);
void stats_report_init(StatsReport *report);

void stats_add_original_bytes(StatsReport *report, uint64_t bytes);
void stats_add_compressed_bytes(StatsReport *report, uint64_t bytes);
void stats_add_written_bytes(StatsReport *report, uint64_t bytes);
void stats_add_write_calls(StatsReport *report, uint64_t calls);

void stats_timer_start(StatsTimer *timer);
void stats_timer_stop(StatsTimer *timer, StatsReport *report);

/* Calcula compression_ratio a partir de los bytes ya registrados */
void stats_finalize(StatsReport *report);

#endif /* STATS_H */
