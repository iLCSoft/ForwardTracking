#include "CritRZRatio.h"


using namespace FTrack;

CritRZRatio::CritRZRatio ( double ratioMax ){
   
   
   _ratioMax = ratioMax;  
   
   
   
}


      
bool CritRZRatio::areCompatible( Segment* parent , Segment* child ){
   
   
   
   if (( parent->getAutHits().size() == 1 )&&( child->getAutHits().size() == 1 )){ //a criteria for 1-segments
      
      
      AutHit* a = parent->getAutHits()[0];
      AutHit* b = child-> getAutHits()[0];
      
      float ax = a->getX();
      float ay = a->getY();
      float az = a->getZ();
      
      float bx = b->getX();
      float by = b->getY();
      float bz = b->getZ();
      
      // the square is used, because it is faster to calculate with the squares than with sqrt, which takes some time!
      double ratioSquared = ( (ax-bx)*(ax-bx) + (ay-by)*(ay-by) + (az-bz)*(az-bz) ) / ( (az-bz) * ( az-bz ) );
            
      
      
      _map_name_value.insert( std::pair < std::string , float > ( "RZRatioSquared", ratioSquared ) );
      
      if ( ratioSquared > _ratioMax * _ratioMax ) return false;
  
      
      
   }
   
   
   return true;
   
   
   
}