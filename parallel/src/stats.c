#include "stats.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STATS_HISTORY_INITIAL_CAPACITY 4096

bool initStats(TimeStats *stats) {
    stats->windowHead = 0;
    stats->windowCount = 0;
    stats->historyCount = 0;
    stats->historyCapacity = STATS_HISTORY_INITIAL_CAPACITY;
    stats->history =
        (double *)malloc(stats->historyCapacity * sizeof(double));
    if (stats->history == NULL)
        return false;

    return true;
}

void pushStat(TimeStats *stats, double value) {
    // fenêtre glissante : on écrase la valeur la plus ancienne
    stats->window[stats->windowHead] = value;
    stats->windowHead = (stats->windowHead + 1) % STATS_WINDOW_SIZE;
    if (stats->windowCount < STATS_WINDOW_SIZE)
        stats->windowCount++;

    // historique : on double la capacité quand il est plein
    if (stats->historyCount == stats->historyCapacity) {
        size_t newCapacity = stats->historyCapacity * 2;
        double *newHistory =
            (double *)realloc(stats->history, newCapacity * sizeof(double));
        if (newHistory == NULL)
            return;
        stats->history = newHistory;
        stats->historyCapacity = newCapacity;
    }
    stats->history[stats->historyCount++] = value;
}

static int compareDouble(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    return (da > db) - (da < db);
}

// méthode du rang le plus proche (nearest-rank) sur un tableau trié
static double percentile(const double *sorted, size_t n, double p) {
    size_t rank = (size_t)ceil(p / 100.0 * n);
    if (rank < 1)
        rank = 1;
    return sorted[rank - 1];
}

// attention : trie values sur place
static StatsSummary computeSummary(double *values, size_t n) {
    StatsSummary summary = {0};
    summary.count = n;
    if (n == 0)
        return summary;

    qsort(values, n, sizeof(double), compareDouble);

    double sum = 0.0;
    for (size_t i = 0; i < n; i++) {
        sum += values[i];
    }

    summary.mean = sum / n;
    summary.min = values[0];
    summary.max = values[n - 1];
    summary.p50 = percentile(values, n, 50.0);
    summary.p95 = percentile(values, n, 95.0);
    summary.p99 = percentile(values, n, 99.0);

    return summary;
}

StatsSummary getWindowStats(const TimeStats *stats) {
    double sorted[STATS_WINDOW_SIZE];
    memcpy(sorted, stats->window, stats->windowCount * sizeof(double));
    return computeSummary(sorted, stats->windowCount);
}

StatsSummary getGlobalStats(const TimeStats *stats) {
    StatsSummary summary = {0};
    if (stats->historyCount == 0)
        return summary;

    double *sorted = (double *)malloc(stats->historyCount * sizeof(double));
    if (sorted == NULL)
        return summary;
    memcpy(sorted, stats->history, stats->historyCount * sizeof(double));
    summary = computeSummary(sorted, stats->historyCount);
    free(sorted);

    return summary;
}

void printStatsHeader(void) {
    printf("%-12s %8s %9s %9s %9s %9s %9s %9s\n", "(ms)", "n", "avg", "min",
           "P50", "P95", "P99", "max");
}

void printStats(const char *name, const TimeStats *stats) {
    StatsSummary s = getGlobalStats(stats);
    printf("%-12s %8zu %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f\n", name, s.count,
           s.mean, s.min, s.p50, s.p95, s.p99, s.max);
}

void destroyStats(TimeStats *stats) {
    free(stats->history);
    stats->history = NULL;
    stats->historyCount = 0;
    stats->historyCapacity = 0;
}
