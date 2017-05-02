#include "DCT.h"
#include "BandPassFilters.h"
#include "LowPassFilters.h"
#include "WaveGenerator.h"
#include "CordicQueueIntegrator.h"

#include <stdint.h>
#include <stdio.h>
#include <complex>

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
  double fc2 = 150;  // Data
  double fs = 1000;  // Sampling frequency
  double lock_high_threshold = 0.8;
  double lock_low_threshold = 0.6;

  uint32_t data_cycles = fs/fc1*20;
  uint32_t transition_cycles = fs/fc1*5;
  
  CarrierGenerator carrier( fc1/fs, 1.0 );
  PhaseWaveGenerator dataw( fc2/fs, 
			      [data_cycles,transition_cycles]( PhaseWaveGenerator::Cycle& c ) {
                      c.transition_cycles = transition_cycles;
                      c.data_cycles = data_cycles;
                      c.amplitude  = 1.0;
                      c.phase = c.phase>0 ? 0 : M_PI;
			      });
 
  double bw  = (fc2-fc1)/2;
  BandPassFilter bp1( fc1/fs, bw/fs, 4 );
  BandPassFilter bp2( fc2/fs, bw/fs, 4 );
  
  CordicQueueIntegrator it1slow( transition_cycles, fc1/fs );
  CordicQueueIntegrator it2slow( transition_cycles, fc2/fs );
  CordicQueueIntegrator it1fast( transition_cycles/2, fc1/fs );
  CordicQueueIntegrator it2fast( transition_cycles/2, fc2/fs );
    
  CordicIntegrator ic1( fc1 );
  CordicIntegrator ic2( fc2 );
  
  uint32_t lock_cycle_start;
  double max_lock;
  bool locked = false;
  double dt = 1/fs;
  for ( unsigned j=0; j<100*4*data_cycles; ++j ) 
  {
    double t = j*dt;
    double signal = carrier.step() + dataw.step();
    double sig1 = bp1.add( signal );
    double sig2 = bp2.add( signal );
    it1slow.add( sig1 );
    it2slow.add( sig2 );
    it1fast.add( sig1 );
    it2fast.add( sig2 );
    //printf( "Phase: %3.0f %3.0f   Level: %f %f \n", it1.phase()*180/M_PI, it2.phase()*180/M_PI, it1.level(), it2.level() );        
  
    if ( j>transition_cycles ) {
        double slow_lock = it2slow.level()/it1slow.level();
        double fast_lock = it2fast.level()/it1fast.level();
        if ( !locked ) {
            if ( slow_lock>lock_high_threshold ) {
                if ( (fast_lock >= slow_lock) && (fast_lock<1.1*slow_lock) ) {      
                    locked = true;
                    ic1.reset();
                    ic2.reset();
                    max_lock = slow_lock;
                    lock_cycle_start = j;
                    printf( "LOCK-IN  Cycle: %3d  Phase: %3.0f %3.0f   Lock: %f %f\n", j, it1slow.phase()*180/M_PI, it2slow.phase()*180/M_PI, slow_lock, fast_lock );        
                }
            }
        }
        else {
            // locked
            // keep the maximum lock
            if ( slow_lock > max_lock ) max_lock = slow_lock;
            // decide when it lost the lock
            double in_lock = ic2.level()/ic1.level();
            if ( (fast_lock<0.9*max_lock) || (fast_lock<in_lock) ) {
                locked = false;
                printf( "LOCK-OUT Cycle: %3d Phase: %3.0f %3.0f   Lock: %f %f   In-Lock:%f  In-Phase:%3.0f %3.0f\n", 
                j-lock_cycle_start, it1slow.phase()*180/M_PI, it2slow.phase()*180/M_PI, 
                slow_lock, fast_lock, in_lock, ic1.phase()*180/M_PI, ic2.phase()*180/M_PI );        
            }
        }
        if ( locked ) {
            ic1.add( sig1 );
            ic2.add( sig2 );
        }
    }
    
  }

}

