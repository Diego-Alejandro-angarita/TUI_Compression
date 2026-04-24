#ifndef STATS_H
#define STATS_H

#include <stddef.h>

typedef struct {
    size_t bytes_written_classic;
    size_t bytes_written_proposed;
    size_t write_calls_classic;
    size_t write_calls_proposed;
    double user_time_ms;
    double system_time_ms;
    double wall_time_ms;
} StatsReport;

void stats_report_init(StatsReport *report);
void stats_begin(StatsReport *report);
void stats_end(StatsReport *report);

#endif

