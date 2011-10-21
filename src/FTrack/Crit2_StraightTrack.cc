#include "Crit2_StraightTrack.h"



using namespace FTrack;

   Crit2_StraightTrack::Crit2_StraightTrack ( double ratioMax ){
   
   
   _ratioMax = ratioMax;  
   
   _saveValues = false;
   
}


      
      bool Crit2_StraightTrack::areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength ){
   
   
   
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
      if (_saveValues){
         _map_name_value["StraighTrack_rhoParent"] = sqrt( rhoASquared );
         _map_name_value["StraighTrack_rhoChild"]  = sqrt( rhoBSquared );
         _map_name_value["StraighTrack_rhoParent-rhoChild"] = sqrt( rhoASquared ) - sqrt( rhoBSquared );
      }
      
      

      
      if (_saveValues){
         _map_name_value["StraightTrack_Ratio"]= 0.;
         
      }
      
      if (rhoBSquared > rhoASquared ) return false;
      
      
      if( (rhoBSquared >0.) && ( az != 0. ) ){ //prevent division by 0
         
         // the square is used, because it is faster to calculate with the squares than with sqrt, which takes some time!
         double ratioSquared = ( ( rhoASquared * ( bz*bz )  ) / ( rhoBSquared * ( az*az )  ) );
               
         if (_saveValues) _map_name_value["StraightTrack_Ratio"] = sqrt(ratioSquared);
         
         
         if ( ratioSquared > _ratioMax * _ratioMax ) return false;
   
         if ( ratioSquared < _ratioMin * _ratioMin ) return false;
      
      }
      
   }
   else{
      
      std::string s = "Crit2_StraightTrack::This criterion needs 2 segments with 1 hit each, passed was a "
      +  intToString( parent->getAutHits().size() ) + " hit segment (parent) and a "
      +  intToString( child->getAutHits().size() ) + " hit segment (child).";

      
      throw BadSegmentLength( s );
      
      
   }
   
   
   return true;
   
   
   
}


