#pragma once
#include "LowPassFilters.h"

struct LockDetector {
    LockDetector( double fs, double threshold = 0.5 ) 
      : in_phase_lp(0.707106, 10.0, fs),
        qu_phase_lp(0.707106, 10.0, fs)
    {
        thresh = threshold;
    }

    void reset() {
        in_phase_lp.reset();
        qu_phase_lp.reset();
    }

    bool add( double in_phase, double qu_phase) {
        in_phase = in_phase_lp.add(in_phase*in_phase);
        qu_phase = qu_phase_lp.add(qu_phase*qu_phase);
        if (in_phase > thresh && qu_phase < thresh)
            return true;
        return false;
    }

private:
    BiquadLowPassFilter in_phase_lp;
    BiquadLowPassFilter qu_phase_lp;
    double thresh;
};