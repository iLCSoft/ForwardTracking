#ifndef Crit4_distToExtrapolation_h
#define Crit4_distToExtrapolation_h


#include "ICriterion.h"

namespace FTrack{
   
   /** Criterion: check for the change of the 2D angle
    */
   class Crit4_distToExtrapolation : public ICriterion{
      
      
      
   public:
      
      /**
       * @param distMax 
       */
      Crit4_distToExtrapolation ( double distMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength );
      
      virtual ~Crit4_distToExtrapolation(){};
      
   private:
      
      double _distMax;
      
   };
   
}

#endif
