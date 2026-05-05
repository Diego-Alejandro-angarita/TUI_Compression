#define _POSIX_C_SOURCE 200809L

#include "stats.h"

#include <string.h>
#include <sys/resource.h>
#include <time.h>

static double ts_ms(const struct timespec *time)
{
    return (double)time->tv_sec * 1000.0 + (double)time->tv_nsec / 1000000.0;
}

static double tv_ms(const struct timeval *time)
{
    return (double)time->tv_sec * 1000.0 + (double)time->tv_usec / 1000.0;
}

void stats_init(StatsReport *report)
{
    if (report == NULL) {
        return;
    }

    memset(report, 0, sizeof(*report));
}

void stats_report_init(StatsReport *report)
{
    stats_init(report);
}

void stats_add_original_bytes(StatsReport *report, uint64_t bytes)
{
    if (report != NULL) {
        report->bytes_original += bytes;
    }
}

void stats_add_compressed_bytes(StatsReport *report, uint64_t bytes)
{
    if (report != NULL) {
        report->bytes_compressed += bytes;
    }
}

void stats_add_written_bytes(StatsReport *report, uint64_t bytes)
{
    if (report != NULL) {
        report->bytes_written += bytes;
    }
}

void stats_add_write_calls(StatsReport *report, uint64_t calls)
{
    if (report != NULL) {
        report->write_calls += calls;
    }
}

void stats_timer_start(StatsTimer *timer)
{
    if (timer == NULL) {
        return;
    }

    memset(timer, 0, sizeof(*timer));

    struct timespec wall_start;
    struct rusage usage_start;

    clock_gettime(CLOCK_MONOTONIC, &wall_start);
    getrusage(RUSAGE_SELF, &usage_start);

    timer->wall_start_ms = ts_ms(&wall_start);
    timer->cpu_user_start_ms = tv_ms(&usage_start.ru_utime);
    timer->cpu_sys_start_ms = tv_ms(&usage_start.ru_stime);
}

void stats_timer_stop(StatsTimer *timer, StatsReport *report)
{
    if (timer == NULL || report == NULL) {
        return;
    }

    struct timespec wall_end;
    struct rusage usage_end;

    clock_gettime(CLOCK_MONOTONIC, &wall_end);
    getrusage(RUSAGE_SELF, &usage_end);

    report->wall_clock_ms += ts_ms(&wall_end) - timer->wall_start_ms;
    report->cpu_user_ms += tv_ms(&usage_end.ru_utime)
        - timer->cpu_user_start_ms;
    report->cpu_sys_ms += tv_ms(&usage_end.ru_stime)
        - timer->cpu_sys_start_ms;
}

void stats_finalize(StatsReport *report)
{
    if (report == NULL) {
        return;
    }

    report->compression_ratio = (report->bytes_original > 0)
        ? (double)report->bytes_compressed / (double)report->bytes_original
        : 0.0;
}
