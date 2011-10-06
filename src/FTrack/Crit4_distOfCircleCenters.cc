#include "Crit4_distOfCircleCenters.h"

#include <cmath>
#include "SimpleCircle.h"




using namespace FTrack;

Crit4_distOfCircleCenters::Crit4_distOfCircleCenters ( double distMax ){
   
   
   _distMax = distMax;
   
   
   
}



bool Crit4_distOfCircleCenters::areCompatible( Segment* parent , Segment* child ){
    
   
   
   if (( parent->getAutHits().size() == 3 )&&( child->getAutHits().size() == 3 )){ //this is a criterion for 3-segments
      


      AutHit* a = parent->getAutHits()[0];
      AutHit* b = parent->getAutHits()[1];
      AutHit* c = parent->getAutHits()[2];
      AutHit* d = child-> getAutHits()[2];
      
      float ax = a->getX();
      float ay = a->getY();
//       float az = a->getZ();
      
      float bx = b->getX();
      float by = b->getY();
//       float bz = b->getZ();
      
      float cx = c->getX();
      float cy = c->getY();
//       float cz = c->getZ();
      
      float dx = d->getX();
      float dy = d->getY();
//       float dz = d->getZ();
      
      
      SimpleCircle circle1 ( ax , ay , bx , by , cx , cy );
      SimpleCircle circle2 ( bx , by , cx , cy , dx , dy );

      
      double X1 = circle1.getCenterX();
      double Y1 = circle1.getCenterY();

      double X2 = circle2.getCenterX();
      double Y2 = circle2.getCenterY();
      
      
      float distOfCircleCenters = sqrt( (X2-X1)*(X2-X1) + (Y2-Y1)*(Y2-Y1) );
      
      _map_name_value["distOfCircleCenters"] = distOfCircleCenters;
      
      if ( distOfCircleCenters > _distMax ) return false;
      
      
      
      
   }
   
   
   return true;
   
   
   
}

