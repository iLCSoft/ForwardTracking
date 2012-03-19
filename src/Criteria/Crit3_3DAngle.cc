#include "Crit3_3DAngle.h"

#include <cmath>
#include <sstream>

using namespace FTrack;

Crit3_3DAngle::Crit3_3DAngle ( float angleMin, float angleMax ){
   
   
   _cosAngleMin = cos ( angleMax * M_PI / 180. );
   _cosAngleMax = cos ( angleMin * M_PI / 180. );
   
   _name = "Crit3_3DAngle";
   _type = "3Hit";
   
   _saveValues = false;
   
}


      
bool Crit3_3DAngle::areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength ){
   
   
   //this is not written very beautiful, because this code gets called often and needs to be fast.
   //But it's just a simple angle calculation of two vectors cos(alpha) = u*v/|u||v|
   //
   //Because of speed, I avoided using stuff like sqrt or cos or something in here.
   //That's why it may look a bit odd.
   
   
   
   
   if (( parent->getHits().size() == 2 )&&( child->getHits().size() == 2 )){ //this is a criterion for 2-segments
      


      IHit* a = parent->getHits()[0];
      IHit* b = parent->getHits()[1];
      IHit* c = child-> getHits()[1];
      
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
      
      if (_saveValues){
         
         _map_name_value["Crit3_3DAngle_cos3DAngleSquared"] =  1.;
         _map_name_value["Crit3_3DAngle"] = 0.;
         
      }
      
      if ( denomSquared > 0.){ //don't divide by 0
      
         double cosThetaSquared = numerator * numerator / ( uSquared * vSquared );
         
         if (_saveValues){
            
            _map_name_value["Crit3_3DAngle_cos3DAngleSquared"] =  cosThetaSquared;
            _map_name_value["Crit3_3DAngle"] = acos( sqrt( cosThetaSquared ) ) * 180. / M_PI;
            
         }
         
         if (cosThetaSquared < _cosAngleMin*_cosAngleMin) return false;
         if (cosThetaSquared > _cosAngleMax*_cosAngleMax) return false;
      
      }
      
      
      

      
      
   }
   else{
      
      std::stringstream s;
      s << "Crit3_3DAngle::This criterion needs 2 segments with 2 hits each, passed was a "
      <<  parent->getHits().size() << " hit segment (parent) and a "
      <<  child->getHits().size() << " hit segment (child).";
      
      
      throw BadSegmentLength( s.str() );
      
      
   }
   
   
   return true;
   
   
   
}

