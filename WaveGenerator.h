#pragma once

#include <functional>
#include <complex>
#include <limits>
#include <stdint.h>
#include <math.h>

#include "CordicGenerator.h"


/*******************************************************************
Generates a sine wave with the provided amplitude, phase and frequency
Takes care of C1 continuity transitions
*******************************************************************/
class WaveGenerator {
public:
  struct State {
    double   phase;
    double   amplitude;
    uint32_t steps;
  };
  
  typedef void (FType)( State& );
  
  WaveGenerator( double fc, std::function<FType>&& fn )
    : _fn( fn ), _fc( fc ), _cordic( fc )
  {
    _target.phase = 0;
    _target.amplitude = 1;
    _target.steps = 0;
    _countdown = 0;
    _fn( _target );
    _amp = _target.amplitude;
    _amp_inc = 0;
  }

  double step() {
    if ( _countdown==0 ) {
      recalc();
    }
    double value = _amp*_cordic.real(); 
    advance();
    return value;
  }

  void advance() {
    if ( _countdown>0 ) _countdown--;
    _cordic.advance();
    _amp += _amp_inc;
  }

private:
  void recalc() {
    double old_phase = _target.phase;
    _fn( _target );
    double addl_phase_per_step = 0;
    if ( _target.steps==0 ) {
      addl_phase_per_step = 0;
      _countdown = std::numeric_limits<decltype(_countdown)>::max();
      _amp_inc = 0;
    }
    else {
      addl_phase_per_step = ( _target.phase - old_phase )/_target.steps;
      _countdown = _target.steps;
      _amp_inc = (_target.amplitude - _amp)/_target.steps;
    }
    _cordic.set_freq( _fc + addl_phase_per_step/(2*M_PI) );
  }
  
  double _fc;
  double _amp;
  double _amp_inc;  
  State _target;
  CordicGenerator _cordic;
  //double _cs, _sn, _x, _y;
  double _value;
  uint64_t _countdown;
  std::function< FType > _fn;
};

/*******************************************************************
  Generates a simple sine wave
*******************************************************************/
class CarrierGenerator : public WaveGenerator {
public:
  CarrierGenerator( double fc, double amplitude )
  : WaveGenerator( fc, [amplitude]( WaveGenerator::State& t ) {
      t.steps = 0;
      t.phase = 0;
      t.amplitude = amplitude;
    } ) {};
};

/*******************************************************************
  Generates a wave with the phase supplied
*******************************************************************/
class PhaseWaveGenerator : public WaveGenerator {
public:
  struct Cycle {
    uint32_t transition_cycles;
    uint32_t data_cycles;
    double   amplitude;
    double   phase;
  };
  typedef void (CycleGen)( Cycle& );
  PhaseWaveGenerator( double fc, std::function<CycleGen>&& gen )
    : WaveGenerator( fc, [gen,this]( WaveGenerator::State& t ) {
	switch ( _stage ) { 
	case 0:  // trans -> ON
	  gen( _state );
	  t.steps = _state.transition_cycles;
	  t.phase = _state.phase;
	  t.amplitude = _state.amplitude;
	  _stage = 1;
	  break;
	case 1:  // ON -> trans	  
	  t.steps = _state.data_cycles;
	  t.phase = _state.phase;
	  t.amplitude = _state.amplitude;
	  _stage = 0;
	  break;
	}
      })
  {
    _stage = 0;
  }
private:
  Cycle _state;
  uint32_t _stage;
};



/**
Generates a waves that carries binary data
*/
class DataWaveGenerator : public PhaseWaveGenerator {
public:
  struct Data {
    uint32_t bits;
    uint32_t val;
  };
  typedef void (DataGen)( Data& );
  DataWaveGenerator( double fc,
		      double amplitude,
		      uint32_t transition_cycles,
		      uint32_t data_on_cycles,
		      uint32_t data_off_cycles,
		      std::function<DataGen>&& gen )
    : PhaseWaveGenerator( fc,
       [gen,this,amplitude,transition_cycles,data_on_cycles,data_off_cycles]
			       ( PhaseWaveGenerator::Cycle& c ) {
	c.amplitude = amplitude;
	c.transition_cycles = transition_cycles;
	if ( _off_cycle ) {
	  if ( _data.bits == 0 ) {	    
	    gen( _data );
	  }	    
	  c.data_cycles = data_off_cycles;
	  c.phase = (_data.val & 1) == 0 ?  0 : M_PI;
	  _data.val >>= 1;
	  _data.bits--;
	  _off_cycle = false;
	}
	else { 
	  c.data_cycles = data_on_cycles;
	  _off_cycle = true;
	}
	return true;
      })
  {
    _data.bits = 0;
    _data.val = 0;
    _off_cycle = true;
  }
private:
  Data     _data;
  bool     _off_cycle;
};

