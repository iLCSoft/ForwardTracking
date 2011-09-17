#include "Crit2_StraightTrack.h"


using namespace FTrack;

   Crit2_StraightTrack::Crit2_StraightTrack ( double ratioMax ){
   
   
   _ratioMax = ratioMax;  
   
   
   
}


      
bool Crit2_StraightTrack::areCompatible( Segment* parent , Segment* child ){
   
   
   
   if (( parent->getAutHits().size() == 1 )&&( child->getAutHits().size() == 1 )){ //a criterion for 1-segments
      
      
      
      AutHit* a = parent->getAutHits()[0];
      AutHit* b = child-> getAutHits()[0];
      
      float ax = a->getX();
      float ay = a->getY();
      float az = a->getZ();

      float bx = b->getX();
      float by = b->getY();
      float bz = b->getZ();
      
      //the distance to (0,0) in the xy plane
      double rhoASquared = ax*ax + ay*ay;
      double rhoBSquared = bx*bx + by*by;
      
      //first check, if the distance to (0,0) rises --> such a combo could not reach the IP
      _map_name_value.insert( std::pair < std::string , float > ( "rhoParentSquared", rhoASquared ) );
      _map_name_value.insert( std::pair < std::string , float > ( "rhoChildSquared",  rhoBSquared ) );
      

      if (rhoBSquared > rhoASquared ) return false;
      
      
      if( (rhoBSquared >0.) && ( az != 0. ) ){ //prevent division by 0
         
         // the square is used, because it is faster to calculate with the squares than with sqrt, which takes some time!
         double ratioSquared = ( ( rhoASquared * ( bz*bz )  ) / ( rhoBSquared * ( az*az )  ) );
               
         _map_name_value.insert( std::pair < std::string , float > ( "StraightTrackRatioSuqared", ratioSquared ) );
         
         
         if ( ratioSquared > _ratioMax * _ratioMax ) return false;
   
         if ( ratioSquared < 1./ ( _ratioMax * _ratioMax ) ) return false;
      
      }
      
   }
   
   
   return true;
   
   
   
}