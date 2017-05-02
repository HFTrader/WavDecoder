#pragma once
#include <stdint.h>

struct Integrator {
    Integrator(double fs) {
        sum = 0.0;
        twofs = 2.0 * fs;
    }

    double add(double input) {
        long double out = input + sum;
        sum = input + out;
        return out/twofs;
    }

    double value() {
        return (double) (sum/twofs);
    }

    void reset() {
        sum = 0;
    }

private:
    long double sum;
    double twofs;
};

