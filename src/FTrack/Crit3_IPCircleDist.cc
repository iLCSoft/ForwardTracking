#include "Crit3_IPCircleDist.h"

#include "SimpleCircle.h"
#include <cmath>

using namespace FTrack;


Crit3_IPCircleDist::Crit3_IPCircleDist( double distToCircleMax ){
   
   _distToCircleMax  = distToCircleMax;
   
   
}



bool Crit3_IPCircleDist::areCompatible( Segment* parent , Segment* child ){
   
   
   
   if (( parent->getAutHits().size() == 2 )&&( child->getAutHits().size() == 2 )){ //a criterion for 2-segments


      AutHit* a = parent->getAutHits()[0];
      AutHit* b = parent->getAutHits()[1];
      AutHit* c = child-> getAutHits()[1];
      
      float ax = a->getX();
      float ay = a->getY();
     
      float bx = b->getX();
      float by = b->getY();
      
      float cx = c->getX();
      float cy = c->getY();



      SimpleCircle circle ( ax , ay , bx , by , cx , cy );
      
      double x = circle.getCenterX();
      double y = circle.getCenterY();
      double R = circle.getRadius();
      
      double circleDistToIP = fabs( R - sqrt (x*x+y*y) );
      
      _map_name_value["circleDistToIP"] =  circleDistToIP;
            
      if ( circleDistToIP  > _distToCircleMax ) return false;






   }
   
   
   
   return true;
   
   
}
