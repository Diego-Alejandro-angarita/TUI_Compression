#include "profiler.h"

#include <string.h>

void profiler_report_init(ProfileReport *report)
{
    if (report == NULL) {
        return;
    }

    memset(report, 0, sizeof(*report));
}

void profiler_timer_start(ProfileTimer *timer, ProfileStage stage)
{
    if (timer == NULL) {
        return;
    }

    timer->stage = stage;
    timer->start_ms = 0.0;

    /* TODO: Capturar tiempo monotono real cuando se active profiling. */
}

void profiler_timer_stop(ProfileTimer *timer, ProfileReport *report)
{
    if (timer == NULL || report == NULL) {
        return;
    }

    /* TODO: Acumular duracion real por etapa. Por ahora conserva valores dummy. */
    switch (timer->stage) {
        case PROFILE_STAGE_COMPRESSION:
            report->compression_ms += 0.0;
            break;
        case PROFILE_STAGE_CRYPTO:
            report->crypto_ms += 0.0;
            break;
        case PROFILE_STAGE_WRITE:
            report->write_ms += 0.0;
            break;
        case PROFILE_STAGE_TOTAL:
            report->total_ms += 0.0;
            break;
        case PROFILE_STAGE_COUNT:
            break;
    }
}
