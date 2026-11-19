#ifndef INCLUDEsrcstatsstats
#define INCLUDEsrcstatsstats

#include "constants.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    double mean, min, max;
    double p50, p95, p99;
    size_t count;
} StatsSummary;

typedef struct {
    // fenêtre glissante (buffer circulaire) pour l'affichage temps réel
    double window[STATS_WINDOW_SIZE];
    size_t windowHead, windowCount;
    // historique complet pour les stats de fin d'exécution
    double *history;
    size_t historyCount, historyCapacity;
} TimeStats;

bool initStats(TimeStats *stats);
void pushStat(TimeStats *stats, double value);
StatsSummary getWindowStats(const TimeStats *stats);
StatsSummary getGlobalStats(const TimeStats *stats);
void printStatsHeader(void);
void printStats(const char *name, const TimeStats *stats);
void destroyStats(TimeStats *stats);

#endif // INCLUDEsrcstatsstats.h_
