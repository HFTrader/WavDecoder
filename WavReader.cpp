#include "WavFormat.h"
#include "BandPassFilters.h"
#include "LowPassFilters.h"
#include "FileUtils.h"
#include "Integrators.h"
#include "CostasLoop.h"

const uint32_t CARRIER_HZ = 1000;
const uint32_t DATAOFF_HZ = 50;
const uint32_t FADE_CYCLES = 10;
const uint32_t DATA_CYCLES = 20;

bool decodeSound( const SampleArray& wav, ByteArray& out, double SAMPLE_HZ ) 
{
  const uint32_t CARRIER_SAMPLES =  SAMPLE_HZ/CARRIER_HZ;
  uint32_t counter = 0;
  uint32_t cycle = 0;
  CostasLoop costas( CARRIER_HZ/SAMPLE_HZ );
  for ( uint32_t j=0; j<wav.size(); ++j ) {
    double sample = double(wav[j])/65536;
    costas.add( sample );
    if ( ++counter >= CARRIER_SAMPLES ) {
      counter -= CARRIER_SAMPLES;
      printf( "Cycle:%4d  Freq:%5.1f  Phase:%3.0f Error:%f\n",
	      cycle++, costas.freq*SAMPLE_HZ, costas.phase*180/M_PI, costas.error );
    }
  }
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
    double freq_hz;

    if ( !readFile( argv[1], bufin ) ) return 1;
    if ( !decodeWavFormat( bufin, samples, freq_hz ) ) return 2;
    if ( !decodeSound( samples, bufout, freq_hz ) ) return 3;
    if ( !writeFile( argv[2], bufout ) ) return 4;

    return 0;
}
