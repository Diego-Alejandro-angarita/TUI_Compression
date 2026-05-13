#ifndef PROFILER_H
#define PROFILER_H

typedef enum {
    PROFILE_STAGE_COMPRESSION = 0,
    PROFILE_STAGE_CRYPTO,
    PROFILE_STAGE_WRITE,
    PROFILE_STAGE_TOTAL,
    PROFILE_STAGE_COUNT
} ProfileStage;

typedef struct {
    double compression_ms;
    double crypto_ms;
    double write_ms;
    double total_ms;
} ProfileReport;

typedef struct {
    ProfileStage stage;
    double start_ms;
} ProfileTimer;

void profiler_report_init(ProfileReport *report);
void profiler_timer_start(ProfileTimer *timer, ProfileStage stage);
void profiler_timer_stop(ProfileTimer *timer, ProfileReport *report);

#endif /* PROFILER_H */
