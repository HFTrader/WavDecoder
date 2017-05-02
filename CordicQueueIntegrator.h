#pragma once

#include "Integrators.h"
#include "CordicGenerator.h"

class CordicQueueIntegrator
{
public: 
  CordicQueueIntegrator( uint32_t samples, double fc )
  {
    _cordic.init( fc );
    numsamples = samples;
    phaseinc = 2*M_PI*fc;
    counter = 0;
    sin_sum = 0;
    cos_sum = 0;
    sin_samples.resize( numsamples, 0 );
    cos_samples.resize( numsamples, 0 );    
  }
  void add( double sample ) {
    double sinval = _cordic.real()*sample;
    double cosval = _cordic.imag()*sample;
    sin_sum += sinval - sin_samples[counter];
    cos_sum += cosval - cos_samples[counter];
    sin_samples[counter] = sinval;
    cos_samples[counter] = cosval;
    if ( ++counter >= numsamples ) counter = 0;
    _cordic.advance();
  }
  double phase() const {
    return ::atan2( sin_sum, cos_sum );
  }
  double level() const {
    return ::sqrt( (sin_sum*sin_sum + cos_sum*cos_sum)/numsamples );
  }
  
private:
  CordicGenerator _cordic;
  double phaseinc;
  double sin_sum;
  double cos_sum;
  uint32_t numsamples;
  std::vector<double> sin_samples;
  std::vector<double> cos_samples;
  uint32_t counter;
};


struct CordicIntegrator
{
  CordicIntegrator( double fc )
  {
    _cordic.init( fc );
    reset();
  }
  
  void reset() {
    counter = 0;
    sin_sum = 0;
    cos_sum = 0;
  }
  
  void add( double sample ) {
    double sinval = _cordic.real()*sample;
    double cosval = _cordic.imag()*sample;
    sin_sum += sinval;
    cos_sum += cosval;
    ++counter;
    _cordic.advance();
  }
  
  double phase() const {
    return ::atan2( sin_sum, cos_sum );
  }
  double level() const {
    return ::sqrt( (sin_sum*sin_sum + cos_sum*cos_sum)/counter );
  }
  
private:
  CordicGenerator _cordic;
  double sin_sum;
  double cos_sum;
  uint32_t counter;
};
