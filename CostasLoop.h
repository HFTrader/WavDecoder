#pragma once
#include "Integrators.h"
#include "LowPassFilters.h"
#include "LockDetector.h"

//https://arxiv.org/pdf/1511.04435.pdf

struct CostasLoop 
{
    CostasLoop( 
        double fc_hz,          // Carrier frequency (normalized for fs=1)
        double qual = -1,      // Quality factor of in and out of phase low pass filters
        double fcut = -1,      // Cutoff frequency for the in and out of phase lpf [Hz] 
                               // Determined by separate program which I will also include 
                               // In CostasLoop directory - it was written in Python
        double fnat = -1,      // Natural Frequency of this control system [Hz]
        double lpcut = -1,     // Cutoff frequency for frequency smoothing
        double zeta = -1       // Damping ratio of this control system
    ) 
    : fc(fc_hz), 
      amp(1.0), vco(1.0),
      lock_detector(fc_hz),
      lock_rc(0.01*fc_hz,1.0)
    {
        // TODO - these parameters need to be better calculated
        if ( qual<0 ) qual = 1.0/sqrt(2.0);
        if ( zeta<0 ) zeta = 1.0/sqrt(2.0);
        if ( fcut<0 ) fcut = 0.6*fc;
        if ( fnat<0 ) fnat = 0.2*fc;
        if ( lpcut<0 ) lpcut = 0.1*fc;

        ilp.init(qual,fcut);
        qlp.init(qual,fcut); 
        flp.init(qual,lpcut);

        // The variable G and a so that the loop achieves the above responses
        G = 4.0*M_PI*zeta*fnat;
        a = fnat*M_PI/zeta;
        inc = 2.0*M_PI*fc;
    
        // Variables to help generate measure frequency
        last_vco_phase = 0.0;
        HZ_PER_RAD = 1.0/2.0/M_PI;
	free_phase = 0;

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
	free_phase += inc;
	phase = vco_phase - free_phase;
	if ( phase>=2*M_PI ) {
	  uint64_t n = phase/(2*M_PI);
	  phase -= n*2*M_PI;
	  free_phase += n*2*M_PI;
	  printf( "Adjusting forward %ld periods\n", n );
	}
	else if ( phase<0 ) {
	  uint64_t n = -phase/(2*M_PI) + 1;
	  phase += n*2*M_PI;
	  free_phase -= n*2*M_PI;
	  printf( "Adjusting backward %ld periods\n", n );
	}
	
	  
	int64_t n = (phase-M_PI)/M_PI;
	if ( n<0 || n>=2 ) { 
	  phase -= n*M_PI;
	  free_phase += n*2*M_PI;
	}

        double lockval = lock_detector.add(in_phase, qu_phase);
        lock = lock_rc.add(lockval);        

        double phase_derivative = (vco_phase - last_vco_phase) * HZ_PER_RAD;
        freq = flp.add(phase_derivative);

	//printf( "Input:%7.3f vco:%5.0f  s/c:%8.6f %8.6f  Phase: %7.6f %7.6f\n",
	//	input, vco_phase*180/M_PI, cos_vco, sin_vco, in_phase, qu_phase );
	//printf( "Input:%7.3f s2:%f s3:%f s4:%f s5:%f s6:%f Phase:%f Freq:%f\n",
	//	input, s2, s3, s4, s5, s6, phase*180/M_PI, freq );
	
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
    double fc;
    double G;
    double a;
    double inc;
    double HZ_PER_RAD;
    
    // Outputs
    double error;
    double lock;
    double freq;
    double phase;
    double free_phase;
    
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
