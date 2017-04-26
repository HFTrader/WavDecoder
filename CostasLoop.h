#pragma once
#include "Integrators.h"
#include "LowPassFilters.h"
#include "LockDetector.h"

//https://arxiv.org/pdf/1511.04435.pdf

struct CostasLoop 
{
    CostasLoop( 
        double fc_hz, // Carrier frequency
        double fs_hz, // Sampling frequency
        double qual = -1,      // Quality factor of in and out of phase low pass filters
        double fcut = -1,          // Cutoff frequency for the in and out of phase lpf [Hz] 
                                          // Determined by separate program which I will also include 
                                          // In CostasLoop directory - it was written in Python
        double fnat = -1,          // Natural Frequency of this control system [Hz]
        double lpcut = -1,         // Cutoff frequency for frequency smoothing
        double zeta = -1  // Damping ratio of this control system
    ) 
    : fc(fc_hz), fs(fs_hz), 
      amp(fs_hz), vco(fs_hz),
      lock_detector(fs_hz),
      lock_rc(0.01*fc_hz,fs_hz)
    {
        if ( qual<0 ) qual = 1.0/sqrt(2.0);
        if ( zeta<0 ) zeta = 1.0/sqrt(2.0);
        if ( fcut<0 ) fcut = 0.6*fc_hz;
        if ( fnat<0 ) fnat = 0.2*fc_hz;
        if ( lpcut<0 ) lpcut = 0.1*fc_hz;

        ilp.init(qual,fcut,fs_hz);
        qlp.init(qual,fcut,fs_hz); 
        flp.init(qual,lpcut,fs_hz );

        // The variable G and a so that the loop achieves the above responses
        G = 4.0*M_PI*zeta*fnat;
        a = fnat*M_PI/zeta;
        inc = 2.0*M_PI*fc;
    
        // Variables to help generate measure frequency
        last_vco_phase = 0.0;
        HZ_PER_RAD = fs/2.0/M_PI;

        // Lock Detector
        reset();
    }   
    
    double add( double input ) {
        // Phase Generator
        // vco_phase is a constantly increasing phase value
        double vco_phase = vco.value();

        // Oscillator
        double cos_vco = cos(vco_phase);
        double sin_vco =-sin(vco_phase);

        // Error Generator
        double in_phase = 2.0*ilp.add(input*cos_vco);
        double qu_phase = 2.0*qlp.add(input*sin_vco);

        // Update Loop Integrators
        double s2 = in_phase*qu_phase;
        double s3 = G * s2;
        double s4 = a * s3;
        double s5 = amp.add(s4);
        double s6 = s3 + s5;
        error = s2;
        vco.add(inc + s6);

        double lockval = lock_detector.add(in_phase, qu_phase);
        lock = lock_rc.add(lockval);        

        double phase_der = (vco_phase - last_vco_phase) * HZ_PER_RAD;
        freq = flp.add(phase_der);

        last_vco_phase = vco_phase;
        return in_phase;    
    }
    
    void reset() {
        last_vco_phase = 0.0;
        ilp.reset();
        qlp.reset();
        vco.reset();
        amp.reset();
        flp.reset();
        lock_detector.reset();
        lock_rc.reset();
    }
    
    // Constants
    double fs;
    double fc;
    double G;
    double a;
    double inc;
    double HZ_PER_RAD;
    
    // Outputs
    double error;
    double lock;
    double freq;
    
    // State
    double last_vco_phase;
    Integrator amp;
    Integrator vco;
    BiquadLowPassFilter ilp;
    BiquadLowPassFilter qlp;
    BiquadLowPassFilter flp;
    LockDetector lock_detector;
    LowPassFilter lock_rc;
};
