#pragma once
#include <stdint.h>
#include <stdio.h>
#include <math.h>

struct LowPassFilter {
    LowPassFilter( double tau, double fs ) {
        tau_fs = tau*fs;
        last = 0.0;
    }

    double add(double sig) {
        last = (sig + tau_fs * last)/(1.0 + tau_fs);
        return last;
    }

    double value() {
        return last;
    }

    void reset() {
        last = 0.0;
    }
    
    double tau_fs;
    double last;
};


struct BiquadLowPassFilter 
{
    BiquadLowPassFilter() {}

    BiquadLowPassFilter(double Q, double fc ) {
        init( Q, fc );
    }
    
    void init( double Q, double fc ) {
        double A = 1.0;
        double omega = 2.0*M_PI*fc;
        double sn = ::sin(omega);
        double cs = ::cos(omega);
        double alpha = sn/(2.0*Q);
        double beta = sqrt(A+A);

        init_priv(A, omega, sn, cs, alpha, beta);
        b0 /= a0;
        b1 /= a0;
        b2 /= a0;
        a1 /= a0;
        a2 /= a0;
    }
    
    double add( double x0 ) {
        double y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        x2 = x1;
        x1 = x0;
        y2 = y1;
        y1 = y0;
        return y0;
    }
    
    void reset() {
        x2 = 0;
        x1 = 0;
        y2 = 0;
        y1 = 0;
    }

    void init_priv(double A, double omega, double sn, double cs, double alpha, double beta) {
        b0 = (1.0 - cs) / 2.0;
        b1 =  1.0 - cs;
        b2 = (1.0 - cs) / 2.0;
        a0 =  1.0 + alpha;
        a1 = -2.0 * cs;
        a2 =  1.0 - alpha;
    }
  
private:
    double a0,a1,a2,b0,b1,b2,x1,x2,y1,y2;
};

