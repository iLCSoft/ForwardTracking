#include "Crit3_3DAngle.h"
#include <cmath>


using namespace FTrack;

Crit3_3DAngle::Crit3_3DAngle ( double angleMax ){
   
   
   _cosAngleMin = cos ( angleMax * M_PI / 180. );
   
   
   
}


      
bool Crit3_3DAngle::areCompatible( Segment* parent , Segment* child ){
   
   
   //this is not written very beautiful, because this code gets called often and needs to be fast.
   //But it's just a simple angle calculation of two vectors cos(alpha) = u*v/|u||v|
   //
   //Because of speed, I avoided using stuff like sqrt or cos or something in here.
   //That's why it may look a bit odd.
   
   
   
   
   if (( parent->getAutHits().size() == 2 )&&( child->getAutHits().size() == 2 )){ //this is a criterion for 2-segments
      


      AutHit* a = parent->getAutHits()[0];
      AutHit* b = parent->getAutHits()[1];
      AutHit* c = child-> getAutHits()[1];
      
      float ax = a->getX();
      float ay = a->getY();
      float az = a->getZ();
      
      float bx = b->getX();
      float by = b->getY();
      float bz = b->getZ();
      
      float cx = c->getX();
      float cy = c->getY();
      float cz = c->getZ();
      
      
      float ux = bx - ax;
      float uy = by - ay;
      float uz = bz - az;
      
      float vx = cx - bx;
      float vy = cy - by;
      float vz = cz - bz;
   
      //In the numerator there is the vector product of u and v   
      double numerator= ux*vx + uy*vy + uz*vz;
      
      //In the denominator there are the lengths of u and v (here squared)
      double uSquared= ux*ux + uy*uy + uz*uz;
      double vSquared= vx*vx + vy*vy + vz*vz;
      

      
      double denomSquared = uSquared * vSquared;
      
      if ( denomSquared > 0.){ //don't divide by 0
      
         double cosThetaSquared = numerator * numerator / ( uSquared * vSquared );
         
         _map_name_value.insert( std::pair < std::string , float > ( "cos3DAngleSquared", cosThetaSquared ) );
         
         if (cosThetaSquared < _cosAngleMin*_cosAngleMin) return false;  
      
      }
      
      
      

      
      
   }
   
   
   return true;
   
   
   
}