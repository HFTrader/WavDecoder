#pragma once


class CordicGenerator
{
    public:
    CordicGenerator() {}
    CordicGenerator( double fc ) { init(fc); }
    void init( double fc ) { 
        set_freq( fc );
        _x = 0;
        _y = 1;
    }
    void advance() {     
        double tx = _x*_cs + _y*_sn;
        double ty = _y*_cs - _x*_sn;
        _x = tx;
        _y = ty;
    }
    void set_freq( double fc ) {
        _sn = sin(2*M_PI*fc);
        _cs = cos(2*M_PI*fc);        
    }
    double real() {
        return _x;
    }
    double imag() { 
        return _y;
    }
    std::complex<double> value() { 
        return std::complex<double>(_x,_y);
    }
    
private:
    double _sn, _cs, _y, _x;
};
