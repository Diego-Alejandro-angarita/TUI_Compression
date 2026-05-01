#define _POSIX_C_SOURCE 200809L

#include "stats.h"

#include <stdio.h>

static int g_run = 0;
static int g_pass = 0;
static int g_fail = 0;

#define T_BEGIN(name) do { g_run++; printf("[TEST] %-54s ", (name)); fflush(stdout); } while (0)
#define T_PASS() do { g_pass++; puts("PASS"); } while (0)
#define T_FAIL(why) do { g_fail++; printf("FAIL - %s\n", (why)); } while (0)

static void test_init_zeroes_report(void)
{
    T_BEGIN("stats_init: todos los campos quedan en cero");

    StatsReport stats;
    stats.bytes_original = 1;
    stats.bytes_compressed = 2;
    stats.bytes_written = 3;
    stats.write_calls = 4;
    stats.cpu_user_ms = 5.0;
    stats.cpu_sys_ms = 6.0;
    stats.wall_clock_ms = 7.0;
    stats.compression_ratio = 8.0;

    stats_init(&stats);

    if (stats.bytes_original == 0
        && stats.bytes_compressed == 0
        && stats.bytes_written == 0
        && stats.write_calls == 0
        && stats.cpu_user_ms == 0.0
        && stats.cpu_sys_ms == 0.0
        && stats.wall_clock_ms == 0.0
        && stats.compression_ratio == 0.0) {
        T_PASS();
    } else {
        T_FAIL("quedaron campos sin limpiar");
    }
}

static void test_add_helpers_accumulate(void)
{
    T_BEGIN("stats_add_*: acumulan contadores");

    StatsReport stats;
    stats_init(&stats);

    stats_add_original_bytes(&stats, 100);
    stats_add_original_bytes(&stats, 23);
    stats_add_compressed_bytes(&stats, 45);
    stats_add_written_bytes(&stats, 50);
    stats_add_write_calls(&stats, 2);
    stats_add_write_calls(&stats, 3);

    if (stats.bytes_original == 123
        && stats.bytes_compressed == 45
        && stats.bytes_written == 50
        && stats.write_calls == 5) {
        T_PASS();
    } else {
        T_FAIL("acumulacion incorrecta");
    }
}

static void test_finalize_ratio(void)
{
    T_BEGIN("stats_finalize: calcula ratio comprimido/original");

    StatsReport stats;
    stats_init(&stats);
    stats_add_original_bytes(&stats, 200);
    stats_add_compressed_bytes(&stats, 50);

    stats_finalize(&stats);

    if (stats.compression_ratio == 0.25) {
        T_PASS();
    } else {
        T_FAIL("ratio incorrecto");
    }
}

static void test_finalize_zero_original(void)
{
    T_BEGIN("stats_finalize: original cero => ratio cero");

    StatsReport stats;
    stats_init(&stats);
    stats_add_compressed_bytes(&stats, 50);

    stats_finalize(&stats);

    if (stats.compression_ratio == 0.0) {
        T_PASS();
    } else {
        T_FAIL("ratio no quedo en cero");
    }
}

static void test_timer_accumulates_non_negative(void)
{
    T_BEGIN("stats_timer: tiempos no negativos");

    StatsReport stats;
    StatsTimer timer;
    stats_init(&stats);

    stats_timer_start(&timer);
    volatile unsigned long sum = 0;
    for (unsigned long i = 0; i < 10000; i++) {
        sum += i;
    }
    (void)sum;
    stats_timer_stop(&timer, &stats);

    if (stats.wall_clock_ms >= 0.0
        && stats.cpu_user_ms >= 0.0
        && stats.cpu_sys_ms >= 0.0) {
        T_PASS();
    } else {
        T_FAIL("tiempos negativos");
    }
}

int main(void)
{
    puts("=== Tests: stats.c (Persona 3) ===\n");

    test_init_zeroes_report();
    test_add_helpers_accumulate();
    test_finalize_ratio();
    test_finalize_zero_original();
    test_timer_accumulates_non_negative();

    printf("\n=== %d/%d pasaron", g_pass, g_run);
    if (g_fail > 0) {
        printf(", %d FALLARON", g_fail);
    }
    puts(" ===\n");

    return (g_fail == 0) ? 0 : 1;
}
