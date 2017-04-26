#pragma once
#include <math.h>
#include <vector>

struct BandPassFilter
{
    struct SOSBandPass 
    {
        SOSBandPass() { }
        SOSBandPass(double a, double b, double c, double d) { init(a,b,c,d); }
        void init(double va, double vb, double vc, double vd) {
            double a0 = vd*vd + vb*vd + vc;
            double a1 = 2*vc - 2*vd*vd;
            double a2 = vd*vd - vb*vd + vc;
            double b0 = va*vd;
            double b1 = 0;
            double b2 =-va*vd;
        
            a[0] = 1.0;
            a[1] = -a1/a0;
            a[2] = -a2/a0;
    
            b[0] = b0/a0;
            b[1] = b1/a0;
            b[2] = b2/a0;

            y[0] = 0.0;
            y[1] = 0.0;
            y[2] = 0.0;

            x[0] = 0.0;
            x[1] = 0.0;
            x[2] = 0.0;
        }
        
        double add(double sig) {
            x[0] = sig;
            // b[1] = 0, so don't worry about it
            y[0] = (x[2] * b[2]) + (x[0] * b[0]) + (y[2] * a[2]) + (y[1] * a[1]);
    
            x[2] = x[1];
            x[1] = x[0];
            y[2] = y[1];
            y[1] = y[0];
            return y[0];
        }
        
        void reset() {
            x[0] = 0.0;
            x[1] = 0.0;
            x[2] = 0.0;
    
            y[0] = 0.0;
            y[1] = 0.0;
            y[2] = 0.0;
        }
        
        double value() {
            return y[0];
        }
        
        double a[3];
        double b[3];
        double y[3];
        double x[3];
    };
    
    struct Poles {
        int order;
        int len;
        double Q[9];
    };
    typedef struct Poles Poles;

    Poles q_poles[8] = {
    {2,  1, {0.71, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00}},
    {4,  2, {0.54, 1.31, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00}},
    {6,  3, {0.52, 0.71, 1.93, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00}},
    {8,  4, {0.51, 0.60, 0.90, 2.56, 0.00, 0.00, 0.00, 0.00, 0.00}},
    {10, 5, {0.51, 0.56, 0.71, 1.10, 3.20, 0.00, 0.00, 0.00, 0.00}},
    {12, 6, {0.50, 0.54, 0.63, 0.82, 1.31, 3.83, 0.00, 0.00, 0.00}},
    {14, 7, {0.50, 0.53, 0.59, 0.71, 0.94, 1.51, 4.47, 0.00, 0.00}},
    {16, 8, {0.50, 0.52, 0.57, 0.65, 0.79, 1.06, 1.72, 5.10, 0.00}} };
    
    static inline double warp(double fc, double fs)
    {
        return 2.0*fs*tan(0.5*2.0*M_PI*fc/fs);
    }

    BandPassFilter( double fs, double fc, double bw, int filter_order ) 
    : order(filter_order) 
    {
        if (order % 2 || order < 2) order = 2;
        double wc = warp(fc, fs);
        double w1 = warp(fc - bw/2.0, fs);
        double w2 = warp(fc + bw/2.0, fs);
        double Q = wc/(w2 - w1);
        double D = 2.0*fs;
        filters.resize(order);
        double q_lp, a, b, q, wo1, wo2, A1, B1, C1, A2, B2, C2;
        Poles& poles( q_poles[order/2-1] );
        int k=0;
        for (int n = 0; n < poles.len; n++)
        {
            q_lp = poles.Q[n];

            a = 1.0/q_lp;
            b = 1.0;
            q = sqrt(Q/a * ((2*Q/a + b/(2*a*Q)) + 
                    sqrt( pow((2*Q/a + b/(2*a*Q)), 2) - 1)));
            wo1 = a*q/(2*Q) + 0.5*sqrt(b/pow(Q, 2) - 1/pow(q, 2));
            wo2 = 1.0/wo1;

            A1 = 1.0/Q * wc;
            B1 = wo1/q * wc;
            C1 = wo1*wo1 * wc*wc;

            A2 = 1.0/Q * wc;
            B2 = wo2/q * wc;
            C2 = wo2*wo2 * wc*wc;

            filters[k++].init(A1,B1,C1,D);
            filters[k++].init(A2,B2,C2,D);
        }
    }
    ~BandPassFilter();

    double add(double sig) {
        for ( auto& f : filters ) sig = f.add(sig);
        return sig;
    }
    
    void reset() {
        for ( auto& f: filters ) f.reset();
    }
    
    double value() {
         return filters[order-1].value();
    }

private:
    int order;
    std::vector<SOSBandPass> filters;
};

