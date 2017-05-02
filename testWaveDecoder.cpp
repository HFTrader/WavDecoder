#include "DCT.h"
#include "BandPassFilters.h"
#include "LowPassFilters.h"
#include "WaveGenerator.h"
#include "CordicQueueIntegrator.h"

#include <stdint.h>
#include <stdio.h>
#include <complex>

static int32_t map_constellation( double phase1, double phase2, uint32_t num_phases ) 
{
    double delta = (2*M_PI)/num_phases;
    double phase_diff = phase2 - phase1;
    int64_t nc = ::llrint( phase_diff/delta );
    //printf( "Phase1:%f Phase2:%f NP:%d delta:%f diff:%f nc:%ld\n", phase1, phase2, num_phases, delta, phase_diff, nc );
    if ( nc<0 ) nc += ((-nc-1)/num_phases + 1)*num_phases;
    if ( nc>=num_phases ) nc = nc % num_phases;    
    return nc;
}

/** Tests carrier phase recovery 
Steps:
1. Generate carrier and phase wave
2. Use band pass filter to separate and recover each
3. Recover phase from carrier
4. Recover phase differential from wave 
*/

int main()
{
  double fc1 = 100;  // Carrier
  double fc2 = 150;  // Clock
  double fc3 = 200;  // Data
  double fs = 1000;  // Sampling frequency
  double lock_high_threshold = 0.8;
  double lock_low_threshold = 0.6;
  uint32_t NBITS = 2;
  uint32_t data_cycles = fs/fc1*20;
  uint32_t transition_cycles = fs/fc1*5;
  
  printf( "Data cycles: %d  Transition cycles: %d\n", data_cycles, transition_cycles );
  
  std::string message = "\0\0Hello World!\0";
  uint32_t msgpos = 0;
  CarrierGenerator carrier( fc1/fs, 1.0 );
  PhaseWaveGenerator clockwav( fc2/fs, 
			      [data_cycles,transition_cycles]( PhaseWaveGenerator::Cycle& c ) {
                      c.transition_cycles = transition_cycles;
                      c.data_cycles = data_cycles;
                      c.amplitude  = 1.0;
                      c.phase += M_PI/2;
                      //if ( c.phase > 2*M_PI ) c.phase -= 2*M_PI;
                      //printf( "Phase now:%f\n", c.phase*180/M_PI );
			      });
  PhaseWaveGenerator datawav( fc3/fs, 
			      [data_cycles,transition_cycles,message,NBITS,&msgpos]( PhaseWaveGenerator::Cycle& c ) {
                      c.transition_cycles = transition_cycles;
                      c.data_cycles = data_cycles;
                      c.amplitude  = 1.0;
                      uint32_t bytepos = msgpos/8;
                      uint32_t bitpos = msgpos%8;
                      uint32_t bit = (uint8_t(message[bytepos]) >> bitpos ) & ((1<<NBITS)-1);
                      double newphase = bit*((2*M_PI)/(1<<NBITS));
                      double diffphase = newphase - c.phase;
                      if ( diffphase>M_PI ) newphase -= 2*M_PI;
                      else if ( diffphase<-M_PI ) newphase += 2*M_PI;
                      c.phase = newphase;
                      printf( "Sending:%d  %f\n", bit, c.phase*180/M_PI );
                      msgpos += NBITS;
			      });
 
  double bw  = (fc2-fc1)/2;
  BandPassFilter bp1( fc1/fs, bw/fs, 4 );
  BandPassFilter bp2( fc2/fs, bw/fs, 4 );
  BandPassFilter bp3( fc3/fs, bw/fs, 4 );
  
  CordicQueueIntegrator it1slow( transition_cycles, fc1/fs );
  CordicQueueIntegrator it2slow( transition_cycles, fc2/fs );
  CordicQueueIntegrator it3slow( transition_cycles, fc3/fs );
  
  CordicQueueIntegrator it1fast( transition_cycles/2, fc1/fs );
  CordicQueueIntegrator it2fast( transition_cycles/2, fc2/fs );
    
  CordicIntegrator ic1( fc3 );
  CordicIntegrator ic3( fc3 );
  
  uint32_t last_state = 0;
  uint32_t phase_up_start =0;
  uint32_t phase_down_start =0;
  bool locked = false;
  
  double dt = 1/fs;
  for ( unsigned j=0; j<1000*4*data_cycles; ++j ) 
  {
    double t = j*dt;
    double signal = carrier.step() + datawav.step() + clockwav.step();
    double sig1 = bp1.add( signal );
    double sig2 = bp2.add( signal );
    double sig3 = bp3.add( signal );
    it1slow.add( sig1 );
    it2slow.add( sig2 );
    it1fast.add( sig1 );
    it2fast.add( sig2 );
    ic3.add( sig3 );
    ic1.add( sig1 );
    //printf( "Phase: %3.0f %3.0f   Level: %f %f \n", it1slow.phase()*180/M_PI, it2slow.phase()*180/M_PI, it1slow.level(), it2slow.level() );        
  
    if ( j>transition_cycles ) {
        double slow_lock = it2slow.level() + it1slow.level();
        double fast_lock = it2fast.level() + it1fast.level();
        if ( !locked ) {
            if ( ( slow_lock>lock_high_threshold ) && (fast_lock >= slow_lock) ) { 
                    locked = true;
                    printf( "LOCK-IN  Cycle: %3d  Phase: %3.0f %3.0f   Lock: %f %f\n", j , it1slow.phase()*180/M_PI, it2slow.phase()*180/M_PI, slow_lock, fast_lock );        
            }
        }
        else {
            // locked
            // decide when it lost the lock
            if ( (slow_lock<lock_low_threshold) && (fast_lock<slow_lock) ) {
                locked = false;
                printf( "LOCK-OUT  Cycle: %3d  Phase: %3.0f %3.0f   Lock: %f %f\n", j , it1slow.phase()*180/M_PI, it2slow.phase()*180/M_PI, slow_lock, fast_lock );        
            }
        }
        
        if ( locked ) 
        {            
            uint32_t ctid = map_constellation( it2slow.phase(), it1slow.phase(), 8 );        
            uint32_t state = ( ctid >> 1 ) + 1;
            if ( (ctid&1)!=0 ) state = 0; 
            
            if ( state!=last_state ) {
                if  ( last_state!=0 ) { // left a valid state = collect data from ic
                    uint32_t ctid = map_constellation( ic3.phase(), ic1.phase(), (1<<NBITS)*2 );
                    uint32_t symbol = ctid>>1;
                    if ( (ctid&1) == 0 ) { 
                        printf( "    symbol: %d  %d  IC1: %f  %f  %f IC2: %f %f \n", symbol, state, ic1.phase()*180/M_PI, ic1.level(), it1fast.phase()*180/M_PI, ic3.phase()*180/M_PI, ic3.level() );
                    }
                    else {
                        printf( "    INVALID symbol: %d  %d  %f  %f\n", ctid, state, ic1.phase()*180/M_PI, ic3.phase()*180/M_PI );                        
                    }
                }
                ic3.reset();
                ic1.reset();
                printf( "Clock transition: %d -> %d, cycle %d\n", last_state, state, j );
                last_state = state;
            }
        }
    }
    
  }

}

