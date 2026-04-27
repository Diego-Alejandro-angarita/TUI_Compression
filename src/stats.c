/*
 * src/stats.c  —  Persona 3: implementación de estadísticas.
 *
 * Este archivo es responsabilidad de Persona 3.
 * Se provee aquí la implementación mínima para que el módulo de
 * compresión (Persona 2) pueda compilar y testearse de forma aislada.
 */

#include "stats.h"
#include <string.h>

void stats_init(StatsReport *report)
{
    if (report)
        memset(report, 0, sizeof(*report));
}

void stats_finalize(StatsReport *report)
{
    if (!report) return;
    report->compression_ratio = (report->bytes_original > 0)
        ? (double)report->bytes_compressed / (double)report->bytes_original
        : 0.0;
}