#include "Crit2_RZRatio.h"
#include "marlin/VerbosityLevels.h"


using namespace FTrack;

Crit2_RZRatio::Crit2_RZRatio ( double ratioMin, double ratioMax ){
   
   
   _ratioMax = ratioMax;
   _ratioMin = ratioMin;
   
   _saveValues = false;
   
   
   
}

  
bool Crit2_RZRatio::areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength ){
   
   
   
   if (( parent->getAutHits().size() == 1 )&&( child->getAutHits().size() == 1 )){ //a criterion for 1-segments
      
      
      AutHit* a = parent->getAutHits()[0];
      AutHit* b = child-> getAutHits()[0];
      
      float ax = a->getX();
      float ay = a->getY();
      float az = a->getZ();
      
      float bx = b->getX();
      float by = b->getY();
      float bz = b->getZ();
      
      // the square is used, because it is faster to calculate with the squares than with sqrt, which takes some time!
      double ratioSquared = 0.; 
      if ( az-bz  != 0. ) ratioSquared = ( (ax-bx)*(ax-bx) + (ay-by)*(ay-by) + (az-bz)*(az-bz) ) / ( (az-bz) * ( az-bz ) );
      
      
      if (_saveValues) _map_name_value[ "RZRatio_RZRatio"] = sqrt( ratioSquared );
      

      
      if ( ratioSquared > _ratioMax * _ratioMax ) return false;
      if ( ratioSquared < _ratioMin * _ratioMin ) return false;
  
      
      
   }
   else{
      
      std::string s = "Crit2_RZRatio::This criterion needs 2 segments with 1 hit each, passed was a "
      +  intToString( parent->getAutHits().size() ) + " hit segment (parent) and a "
      +  intToString( child->getAutHits().size() ) + " hit segment (child).";

      
      throw BadSegmentLength( s );
      
      
   }
   
   
   return true;
   
   
   
}






