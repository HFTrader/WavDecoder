#pragma once

#include "Integrators.h"
#include "CordicGenerator.h"

class CordicQueueIntegrator
{
public: 
  CordicQueueIntegrator( uint32_t num_samples, double fc )
  {
    _cordic.init( fc );
    _num_samples = num_samples;
    _samples.resize( num_samples );
    reset();
  }
  
  void reset() {
    _counter = 0;
    _sin_sum = 0;
    _cos_sum = 0;     
    _sq_sum = 0;
    for ( Pair& p: _samples ) {
        p.sin = p.cos = p.value = 0;
    }
    _ready = false;
  }
  
  void add( double sample ) {
    double sinval = _cordic.real()*sample;
    double cosval = _cordic.imag()*sample;
    Pair& s( _samples[_counter] );
    _sin_sum += sinval - s.sin;
    _cos_sum += cosval - s.cos;
    _sq_sum += sample*sample - s.value*s.value;
    s.sin = sinval;
    s.cos = cosval;
    s.value = sample;
    if ( ++_counter >= _num_samples ) { 
        _ready = true; 
        _counter = 0;
    }
    _cordic.advance();
  }
  
  double phase() const {
    double ph = ::atan2( _sin_sum, _cos_sum );
    if ( ph<0 ) ph += 2*M_PI;
    return ph;
  }
  
  double level() const {
    if ( _ready || (_counter>0) ) 
        return 2*(_sin_sum*_sin_sum + _cos_sum*_cos_sum)/(_num_samples*_sq_sum) ;
    else 
        return 0;
  }
  
  bool ready() const {
      return _ready;
  }
private:
  CordicGenerator _cordic;
  double _sin_sum;
  double _cos_sum;
  double _sq_sum;
  struct Pair { double value; double sin; double cos; };
  std::vector< Pair > _samples;
  uint32_t _counter;
  bool _ready;
  uint32_t _num_samples;
};


struct CordicIntegrator
{
  CordicIntegrator( double fc )
  {
    _cordic.init( fc );
    reset();
  }
  
  void reset() {
    _counter = 0;
    _sin_sum = 0;
    _cos_sum = 0;
    _sq_sum = 0;
  }
  
  void add( double sample ) {
    double sinval = _cordic.real()*sample;
    double cosval = _cordic.imag()*sample;
    _sin_sum += sinval;
    _cos_sum += cosval;
    _sq_sum += sample*sample;
    ++_counter;
    _cordic.advance();
    printf( "-- count:%d sample:%f sin:%f cos:%f sq:%f\n", _counter, sample, _sin_sum, _cos_sum, _sq_sum );
  }
  
  uint32_t count() const {
        return _counter;
  }
  
  double phase() const {
    double ph = ::atan2( _sin_sum, _cos_sum );
    if ( ph<0 ) ph += 2*M_PI;
    return ph;
  }
  double level() const {
    if ( _counter>0 ) 
        return 2*(_sin_sum*_sin_sum + _cos_sum*_cos_sum)/(_counter*_sq_sum);
    return -1;
  }
  
private:
  CordicGenerator _cordic;
  double _sin_sum;
  double _cos_sum;
  double _sq_sum;
  uint32_t _counter;
};
