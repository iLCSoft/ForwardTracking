#ifndef Crit4_DistToExtrapolation_h
#define Crit4_DistToExtrapolation_h


#include "ICriterion.h"

namespace FTrack{
   
   /** Criterion: check for the change of the 2D angle
    */
   class Crit4_DistToExtrapolation : public ICriterion{
      
      
      
   public:
      
      /**
       * @param distMax 
       */
      Crit4_DistToExtrapolation ( float distMin , float distMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength );
      
      virtual ~Crit4_DistToExtrapolation(){};
      
   private:
      
      float _distMin;
      float _distMax;
      
   };
   
}

#endif
