#ifndef Crit2_DeltaPhi_h
#define Crit2_DeltaPhi_h


#include "ICriterion.h"

namespace FTrack{

   /** Criterion: the difference between the phi angles of two hits in grad. 
    */
   class Crit2_DeltaPhi : public ICriterion{



   public:
      
      Crit2_DeltaPhi ( float deltaPhiMin , float deltaPhiMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength );

      virtual ~Crit2_DeltaPhi(){};

    
   private:
      
      float _deltaPhiMax;
      float _deltaPhiMin;
      
      
      
   };

}

#endif

