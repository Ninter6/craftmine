//
// Created by Ninter6 on 2024/7/30.
//

#pragma once

#include <chrono>

struct Ticker {
    using clock = std::chrono::system_clock;

    Ticker() : last(clock::now()) {}
    Ticker(int ms_per_tick, int tick_per_day)
    : last(clock::now()), now(tick_per_day/4*3), ms_per_tick(ms_per_tick), tick_per_day(tick_per_day) {}

    int tick() {
        int pass = (int)std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - last).count();
        if (pass < ms_per_tick) return now;
        last = clock::now();
        now += pass / ms_per_tick;
        if (now > tick_per_day) {
            day += now / tick_per_day;
            now %= tick_per_day;
        }
        day %= 360;
        return now;
    }

    int now{};
    int day{};
    int ms_per_tick{};
    int tick_per_day{};

    clock::time_point last;
};