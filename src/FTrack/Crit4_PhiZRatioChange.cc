#include "Crit4_PhiZRatioChange.h"

#include <cmath>
#include "SimpleCircle.h"
#include "TVector3.h"



using namespace FTrack;

Crit4_PhiZRatioChange::Crit4_PhiZRatioChange ( double changeMax ){
   
   
   _changeMax = changeMax;
   
   _saveValues = false;
   
}



bool Crit4_PhiZRatioChange::areCompatible( Segment* parent , Segment* child ){
    
   
   
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
      float bz = b->getZ();
      
      float cx = c->getX();
      float cy = c->getY();
      float cz = c->getZ();
      
      float dx = d->getX();
      float dy = d->getY();
      float dz = d->getZ();
      
      try{
      
         SimpleCircle circle1 ( ax , ay , bx , by , cx , cy );
         SimpleCircle circle2 ( bx , by , cx , cy , dx , dy );
         
         
         float X1 = circle1.getCenterX();
         float Y1 = circle1.getCenterY();
         float X2 = circle2.getCenterX();
         float Y2 = circle2.getCenterY();
         
         
         
         TVector3 u ( bx - X1, by - Y1 , 0.); //vector from center of circle to point
         TVector3 v ( cx - X1, cy - Y1 , 0.);
         float zDist1 = fabs( cz - bz );
         float phi1 = u.Angle( v );
         float phiZRatio1 = phi1 / zDist1;
         
         TVector3 s ( cx - X2, cy - Y2 , 0.); //vector from center of circle to point
         TVector3 t ( dx - X2, dy - Y2 , 0.);
         float zDist2 = fabs( dz - cz );
         float phi2 = s.Angle( t );
         float phiZRatio2 = phi2 / zDist2;
         
         
         float ratioOfPhiZRatio = phiZRatio1 / phiZRatio2;
         
         if (_saveValues) _map_name_value["PhiZRatioChange_ratioOfPhiZRatio"] = ratioOfPhiZRatio;
         
         
            
         if ( ratioOfPhiZRatio > _changeMax ) return false;    
         if ( 1./ratioOfPhiZRatio > _changeMax ) return false;
         
      }
      catch ( InvalidParameter ){
       
         if (_saveValues) _map_name_value["PhiZRatioChange_ratioOfPhiZRatio"] = 0.;
         
      }
      
         
      
   }
   
   
   return true;
   
   
   
}

