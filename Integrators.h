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

struct QueueIntegrator
{
  QueueIntegrator( uint32_t samples, double phase_increment )
  {
    numsamples = samples;
    phaseinc = phase_increment;
    angle = 0;
    counter = 0;
    sin_sum = 0;
    cos_sum = 0;
    sin_samples.resize( numsamples, 0 );
    cos_samples.resize( numsamples, 0 );    
  }
  void add( double sample ) {
    double sinval = sin( angle )*sample;
    double cosval = cos( angle )*sample;
    sin_sum += sinval - sin_samples[counter];
    cos_sum += cosval - cos_samples[counter];
    sin_samples[counter] = sinval;
    cos_samples[counter] = cosval;
    if ( counter++ >= numsamples ) counter = 0;
    angle += phaseinc;
  }
  double phase() const {
    return ::atan2( sin_sum, cos_sum );
  }
  double level() const {
    return ::sqrt( sin_sum*sin_sum + cos_sum*cos_sum );
  }
  
private:
  double angle;
  double phaseinc;
  double sin_sum;
  double cos_sum;
  uint32_t numsamples;
  std::vector<double> sin_samples;
  std::vector<double> cos_samples;
  uint32_t counter;
};
