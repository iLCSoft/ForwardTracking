#include "Crit4_2DAngleChange.h"
#include <cmath>
#include "TVector3.h"


using namespace FTrack;

Crit4_2DAngleChange::Crit4_2DAngleChange ( double changeMax ){
   
   
   _changeMax = changeMax;
   
   _saveValues = false;
   
}



bool Crit4_2DAngleChange::areCompatible( Segment* parent , Segment* child ){
    
   if (_saveValues) _map_name_value["2DAngleChange_ratioOf2DAngles"] = 0.;
   
   if (( parent->getAutHits().size() == 3 )&&( child->getAutHits().size() == 3 )){ //this is a criterion for 3-segments
      


      AutHit* a = parent->getAutHits()[0];
      AutHit* b = parent->getAutHits()[1];
      AutHit* c = parent->getAutHits()[2];
      AutHit* d = child-> getAutHits()[2];
      
      float ax = a->getX();
      float ay = a->getY();
      float az = a->getZ();
      
      float bx = b->getX();
      float by = b->getY();
      float bz = b->getZ();
      
      float cx = c->getX();
      float cy = c->getY();
      float cz = c->getZ();
      
      float dx = d->getX();
      float dy = d->getY();
      float dz = d->getZ();
      

      
      TVector3 outerVec  (bx-ax , by-ay , bz-az );
      TVector3 middleVec (cx-bx , cy-by , cz-bz );
      TVector3 innerVec  (dx-cx , dy-cy , dz-cz );
      
  
      
      
      double angleXY1 = outerVec.Phi()-middleVec.Phi(); //the angle between 2-segments in the xy plane
      double angleXY2 = middleVec.Phi()-innerVec.Phi();
      
      angleXY1 -= 2*M_PI*floor( angleXY1 /2. /M_PI );    //to the range from 0 to 2pi 
      if (angleXY1 > M_PI) angleXY1 -= 2*M_PI;           //to the range from -pi to pi

      angleXY2 -= 2*M_PI*floor( angleXY2 /2. /M_PI );    //to the range from 0 to 2pi 
      if (angleXY2 > M_PI) angleXY2 -= 2*M_PI;           //to the range from -pi to pi
  
      
      float ratioOf2DAngles = angleXY1 / angleXY2 ;

      if (_saveValues) _map_name_value["2DAngleChange_ratioOf2DAngles"] = ratioOf2DAngles;
      
      if ( ratioOf2DAngles > _changeMax ) return false;    
      if ( 1./ratioOf2DAngles > _changeMax ) return false;
      
         
      
   }
   
   
   return true;
   
   
   
}

