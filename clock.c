#include "clock.h"
#include <stdio.h>

#define ONE_BILLION 1000000000

void increment_clock(struct clock* clock, int increment) {
    clock->nanoseconds += increment;
    if (clock->nanoseconds >= ONE_BILLION) {
        clock->seconds += 1;
        clock->nanoseconds -= ONE_BILLION;
    }
}

struct clock add_clocks(struct clock c1, struct clock c2) {
    struct clock out = {
        .seconds = 0,
        .nanoseconds = 0
    };
    out.seconds = c1.seconds + c2.seconds;
    increment_clock(&out, c1.nanoseconds + c2.nanoseconds);
    return out;
}

int compare_clocks(struct clock c1, struct clock c2) {
    if (c1.seconds > c2.seconds) {
        return 1;
    }
    if ((c1.seconds == c2.seconds) && (c1.nanoseconds > c2.nanoseconds)) {
        return 1;
    }
    if ((c1.seconds == c2.seconds) && (c1.nanoseconds == c2.nanoseconds)) {
        return 0;
    }
    return -1;
}

long double clock_to_seconds(struct clock c) {
    long double seconds = c.seconds;
    long double nanoseconds = (long double)c.nanoseconds / ONE_BILLION; 
    seconds += nanoseconds;
    return seconds;
}

struct clock seconds_to_clock(long double seconds) {
    struct clock clk = { .seconds = (int)seconds };
    seconds -= clk.seconds;
    clk.nanoseconds = seconds * ONE_BILLION;
    return clk;
}

struct clock calculate_avg_time(struct clock clk, int divisor) {
    long double seconds = clock_to_seconds(clk);
    long double avg_seconds = seconds / divisor;
    return seconds_to_clock(avg_seconds);
}

struct clock subtract_clocks(struct clock c1, struct clock c2) {
    long double seconds1 = clock_to_seconds(c1);
    long double seconds2 = clock_to_seconds(c2);
    long double result = seconds1 - seconds2;
    return seconds_to_clock(result);
}

void print_clock(char* name, struct clock clk) {
    printf("%-15s: %'ld:%'ld\n", name, clk.seconds, clk.nanoseconds);
}

struct clock nanoseconds_to_clock(int nanoseconds) {
    // Assumes nanoseconds is less than 2 billion
    struct clock clk = { 
        .seconds = 0, 
        .nanoseconds = 0 
    };

    if (nanoseconds >= ONE_BILLION) {
        nanoseconds -= ONE_BILLION;
        clk.seconds = 1;
    }

    clk.nanoseconds = nanoseconds;
    
    return clk;
}

struct clock get_clock() {
    struct clock out = {
        .seconds = 0,
        .nanoseconds = 0
    };
    return out;
}

void reset_clock(struct clock* clk) {
    clk->seconds = 0;
    clk->nanoseconds = 0;
}