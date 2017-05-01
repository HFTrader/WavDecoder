#pragma once

#include <vector>
#include <stdint.h>
#include <math.h>

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


class DCTArray
{
public:
 DCTArray( unsigned N ) : _dct(N) {}
  DCTArray( const std::vector<double>& freqs ) {
    init( freqs );
  }
  void init( const std::vector<double>& freqs ) {
    uint32_t N = freqs.size();
    _dct.resize( N );
    for ( unsigned j=0; j<N; ++j ) {
      _dct[j].init( freqs[j] );
    }
  }
  void add( double value ) {
    for ( DCT& d : _dct ) {
      d.add( value );      
    }
  }
  DCT& operator []( unsigned j ) {
    return _dct[j];
  }
private:
  std::vector<DCT> _dct;
};
