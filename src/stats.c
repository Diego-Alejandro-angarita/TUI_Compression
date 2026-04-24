#include "stats.h"

#include <string.h>

void stats_report_init(StatsReport *report)
{
    if (report == NULL) {
        return;
    }

    memset(report, 0, sizeof(*report));
}

void stats_begin(StatsReport *report)
{
    if (report == NULL) {
        return;
    }

    stats_report_init(report);
}

void stats_end(StatsReport *report)
{
    (void)report;
}

