#include "Crit3_ChangeRZRatio.h"

#include <cmath>

using namespace FTrack;


Crit3_ChangeRZRatio::Crit3_ChangeRZRatio( double maxChange ){
   
   
   _ratioChangeMaxSquared = maxChange*maxChange;
   
   _saveValues = false;
   
}



bool Crit3_ChangeRZRatio::areCompatible( Segment* parent , Segment* child ){
   
   
   
   if (( parent->getAutHits().size() == 2 )&&( child->getAutHits().size() == 2 )){ //a criterion for 2-segments


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



      // The rz ratios squared
      
      double ratioSquaredParent = 0.; 
      if ( az-bz  != 0. ) ratioSquaredParent = ( (ax-bx)*(ax-bx) + (ay-by)*(ay-by) + (az-bz)*(az-bz) ) / ( (az-bz) * ( az-bz ) );
      
      double ratioSquaredChild = 0.; 
      if ( cz-bz  != 0. ) ratioSquaredChild = ( (cx-bx)*(cx-bx) + (cy-by)*(cy-by) + (cz-bz)*(cz-bz) ) / ( (cz-bz) * ( cz-bz ) );

      double ratioOfRZRatioSquared = 0.;
      
      if (ratioSquaredChild != 0.) ratioOfRZRatioSquared = ratioSquaredParent / ratioSquaredChild;
      
      if (_saveValues) {
         
         _map_name_value["ChangeRZRatio_ratioOfRZRatioSquared"] =  ratioOfRZRatioSquared;
         _map_name_value["ChangeRZRatio_ratioOfRZRatio"] = sqrt( ratioOfRZRatioSquared );
         
      }

      if ( ratioOfRZRatioSquared > _ratioChangeMaxSquared) return false;
      if ( 1. / ratioOfRZRatioSquared > _ratioChangeMaxSquared ) return false;


   }
   
   
   
   return true;
   
   
}
