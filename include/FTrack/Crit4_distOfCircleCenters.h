#ifndef Crit4_distOfCircleCenters_h
#define Crit4_distOfCircleCenters_h


#include "ICriterion.h"

namespace FTrack{
   
   /** Criterion: check for the change of the 2D angle
    */
   class Crit4_distOfCircleCenters : public ICriterion{
      
      
      
   public:
      
      /**
       * @param distMax 
       */
      Crit4_distOfCircleCenters ( double distMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength );
      
      virtual ~Crit4_distOfCircleCenters(){};
      
   private:
      
      double _distMax;
      
   };
   
}

#endif
