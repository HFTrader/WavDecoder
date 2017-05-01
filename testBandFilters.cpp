#include "BandPassFilters.h"
#include "LowPassFilters.h"
#include <stdint.h>
#include <stdio.h>

class DCT
{
public:
  DCT() {};
  DCT( double freq_norm ) { init( freq_norm ); }
  void init(  double fc ) {
    _fc = fc;
    _phase_inc = 2*_fc*M_PI;
    reset();
  }
  void reset() {
    _sum_s = _sum_c = 0;
    _counter = 0;
    _phase = 0;
  }
  void add( double value ) {
    _sum_s += value * sin( _phase );
    _sum_c += value * cos( _phase );
    _phase += _phase_inc;
    _counter++;
  }
  double mag() const {
    return ::sqrt( (_sum_s*_sum_s + _sum_c*_sum_c)/_counter );
  }
  double phase() const {
    return ::atan2( _sum_s, _sum_c );
  }
  double _fc;
  double _sum_s;
  double _sum_c;
  double _phase;
  double _phase_inc;
  uint32_t _counter;
};

int main()
{
  unsigned N = 50;
  double fc = 100;
  double fs = 1000;
  
  std::vector<DCT> dct;
  for ( unsigned  j=0; j<N; ++j ) {
    double fcs = (0.5*j)/N;
    dct.push_back( DCT( fcs ) );
  }

  double Q = 1.0/sqrt(2.0);
  //BiquadLowPassFilter bp( Q, fc, fs );
  BandPassFilter bp( 200/1000., 70/1000., 4 );
  double t = 0;
  double dt = 0.001;
  while ( t<1.0 ) {
    double value = 0;
    for ( unsigned j=0; j<N; ++j ) {
      double fcs = (0.5*j)/N;      
      double w = 2*M_PI*fcs;
      value += cos( w*t );
    }
    double fval = bp.add(value);
    //printf( "%f %f \n", value, fval );
    for ( unsigned  j=0; j<N; ++j ) {
      dct[j].add( fval );
    }
    t += dt;
  }

  for ( unsigned j=0; j<N; ++j ) {
    double fc = (0.5*fs*j)/N;
    printf( "%4d %7.3f %g %7.3f \n", j, fc, dct[j].mag(), dct[j].phase()*180/M_PI );
  }
}

