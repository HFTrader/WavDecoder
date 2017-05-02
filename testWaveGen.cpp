#include "DCT.h"
#include "BandPassFilters.h"
#include "LowPassFilters.h"
#include "WaveGenerator.h"

#include <stdint.h>
#include <stdio.h>
#include <complex>


int main()
{
  unsigned N = 50;
  double fc1 = 100;
  double fc2 = 150;
  double fs = 1000;

  std::vector< double > freqs( N );
  for ( unsigned  j=0; j<N; ++j ) {
    freqs[j] = (0.5*j)/N;
  }

  uint32_t data_cycles = fs/fc1*10;
  uint32_t transition_cycles = fs/fc1*1;
  
  CarrierGenerator carrier( fc1/fs, 1.0 );
  DataWaveGenerator pattern( fc2/fs, 1.0, transition_cycles, data_cycles, data_cycles,
			      []( DataWaveGenerator::Data& d ) {
				d.val = 0b10101010;
				d.bits = 8;
			      });
 
  double Q = 1.0/sqrt(2.0);
  BiquadLowPassFilter bq( Q, 0.5*(fc1+fc2)/fs );
  BandPassFilter bp( (fc1+fc2)/fs, 1.5*(fc2-fc1)/fs, 4 );
  DCTArray dct_bq( freqs );
  DCTArray dct_bp( freqs );
  
  double dt = 1/fs;
  for ( unsigned j=0; j<100*4*data_cycles; ++j ) {
    double t = j*dt;
    double carrier_value = carrier.step();
    double pattern_value = pattern.step();
    double golden = sin( 2*M_PI*fc1*t );
    printf( "Carrier:%9.6f Golden:%9.6f Pattern:%9.6f\n", carrier_value, golden, pattern_value );

    double signal = carrier_value + pattern_value;
    dct_bp.add( bp.add( signal ) );
    dct_bq.add( bq.add( signal ) );
  }

  for ( unsigned j=0; j<N; ++j ) {
    printf( "F:%7.3f   BP-Mag:%7.3f BP-Phase:%8.3f    BQ-Mag:%7.3f BQ-Phase:%8.3f\n",
	    freqs[j]*fs,
	    dct_bp[j].mag(), dct_bp[j].phase()*180/M_PI,
	    dct_bq[j].mag(), dct_bq[j].phase()*180/M_PI );
  }
}

