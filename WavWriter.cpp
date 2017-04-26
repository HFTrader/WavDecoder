#include "WavFormat.h"
#include "BandPassFilters.h"
#include "LowPassFilters.h"
#include "FileUtils.h"
#include "Integrators.h"
#include "CostasLoop.h"


const uint32_t SAMPLE_HZ =  48000;
const uint32_t CARRIER_HZ = 1000;
const uint32_t DATAOFF_HZ = 50;
const uint32_t FADE_CYCLES = 10;
const uint32_t DATA_CYCLES = 20;

const double PHASE_CARRIER_INCR = (2*M_PI*CARRIER_HZ)/SAMPLE_HZ;
const double PHASE_CLOCK_INCR = (2*M_PI*(CARRIER_HZ+DATAOFF_HZ))/SAMPLE_HZ;
const double PHASE_DATA_INCR = (2*M_PI*(CARRIER_HZ+2*DATAOFF_HZ))/SAMPLE_HZ;

 
bool encodeSound( const ByteArray& arr, SampleArray& wav ) 
{
  const uint32_t FADE_SAMPLES = (FADE_CYCLES*SAMPLE_HZ)/CARRIER_HZ;
  const uint32_t DATA_SAMPLES = (DATA_CYCLES*SAMPLE_HZ)/CARRIER_HZ;
  wav.resize( (FADE_SAMPLES+DATA_SAMPLES)*8*arr.size() );

  printf( "Converting %ld bytes into %ld samples\n", arr.size(), wav.size() );
  double phase_carrier = 0;
  double phase_data = 0;
  double phase_data_off  = 0;
  double phase_clock = 0;
  double phase_clock_off = 0;
  uint32_t cnt = 0;
  for ( uint32_t nb = 0; nb<arr.size(); ++nb ) {
      uint32_t byte = arr[nb];
      for ( uint32_t nbits = 0; nbits<8; ++nbits ) {
	// For each bit
	uint32_t bit = (byte>>nbits)&1;
	double phase_data = (bit==0) ? 0 : M_PI;
	double phase_data_incr = (phase_data-phase_data_off)/FADE_SAMPLES;
	double phase_clock_incr = (0-phase_clock_off)/FADE_SAMPLES;
	for ( uint32_t nc =0; nc<FADE_SAMPLES; ++nc ) {
	  double val = (sin( phase_carrier ) + sin( phase_data + phase_data_off ) + sin( phase_clock + phase_clock_off ) );
	  wav[cnt++] = 0.1*(val-0.5)*65536;
	  phase_carrier += PHASE_CARRIER_INCR;
	  phase_clock += PHASE_CLOCK_INCR;
	  phase_data += PHASE_DATA_INCR;
	  phase_data_off += phase_data_incr;
	  phase_clock_off += phase_clock_incr;
	}
	phase_clock_incr = (M_PI-phase_clock_off)/DATA_SAMPLES;
	for ( uint32_t nc =0; nc<DATA_SAMPLES; ++nc ) {
	  double val = (sin( phase_carrier ) + sin( phase_data + phase_data_off ) + sin( phase_clock + phase_clock_off ) );
	  wav[cnt++] = 0.1*(val-0.5)*65536;
	  phase_carrier += PHASE_CARRIER_INCR;
	  phase_clock += PHASE_CLOCK_INCR;
	  phase_data += PHASE_DATA_INCR;
	  phase_clock_off += phase_clock_incr;
	}
      }
    }
    return true;
}


int main( int argc, char* argv[] ) 
{
    if ( argc<=2 ) {
        printf( "Usage: %s <infile> <outfile>\n", argv[0] );
        return 0;
    }

    ByteArray bufin;
    SampleArray samples;
    ByteArray bufout;

    if ( !readFile( argv[1], bufin ) ) return 1;
    if ( !encodeSound( bufin, samples ) ) return 2;
    if ( !encodeWavFormat( samples, bufout, SAMPLE_HZ ) ) return 3;
    if ( !writeFile( argv[2], bufout ) ) return 4;

    return 0;
}
