#include "Crit2_DeltaPhi.h"



using namespace FTrack;

Crit2_DeltaPhi::Crit2_DeltaPhi ( double deltaPhiMin , double deltaPhiMax ){
   
   
   _deltaPhiMax = deltaPhiMax;  
   _deltaPhiMin = deltaPhiMin;  
   
   _saveValues = false;
   
}


      
      bool Crit2_DeltaPhi::areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength ){
   
   
   
   if (( parent->getAutHits().size() == 1 )&&( child->getAutHits().size() == 1 )){ //a criterion for 1-segments
      
      
      
      AutHit* a = parent->getAutHits()[0];
      AutHit* b = child-> getAutHits()[0];
      
      float ax = a->getX();
      float ay = a->getY();


      float bx = b->getX();
      float by = b->getY();

      

      float phia = atan2( ay, ax );
      float phib = atan2( by, bx );
      float deltaPhi = phia-phib;
      if (deltaPhi > M_PI) deltaPhi -= 2*M_PI;           //to the range from -pi to pi
      if (deltaPhi < -M_PI) deltaPhi += 2*M_PI;           //to the range from -pi to pi
      
      if (( by*by + bx*bx < 0.0001 )||( ay*ay + ax*ax < 0.0001 )) deltaPhi = 0.; // In case one of the hits is too close to the origin

      deltaPhi = 180.*fabs( deltaPhi ) / M_PI;
      if (_saveValues) _map_name_value["DeltaPhi_deltaPhi"]= deltaPhi;

      
      
              
         
      if ( deltaPhi > _deltaPhiMax ) return false;

      if ( deltaPhi < _deltaPhiMin ) return false;
      
      
      
   }
   else{
      
      std::string s = "Crit2_DeltaPhi::This criterion needs 2 segments with 1 hit each, passed was a "
      +  intToString( parent->getAutHits().size() ) + " hit segment (parent) and a "
      +  intToString( child->getAutHits().size() ) + " hit segment (child).";

      
      throw BadSegmentLength( s );
      
      
   }
   
   
   return true;
   
   
   
}


