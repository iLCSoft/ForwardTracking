#ifndef SimpleCircle_h
#define SimpleCircle_h

#include "FTrackExceptions.h"

namespace FTrack{

class SimpleCircle {
  
  
  
  public:
  
    
    SimpleCircle ( double x1 , double y1 , double x2 , double y2 , double x3, double y3 ) throw( InvalidParameter );
  
    double getRadius() {return _R;};
    double getCenterX() {return _centerX;};
    double getCenterY() {return _centerY;};
  
  private:
 
  
    double _R;
    double _centerX;
    double _centerY;
    
    double _x1;
    double _x2;
    double _x3;
    
    double _y1;
    double _y2;
    double _y3;
  
  
  
};




} // end of namespace FTrack




#endif


