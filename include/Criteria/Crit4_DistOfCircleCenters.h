#ifndef Crit4_DistOfCircleCenters_h
#define Crit4_DistOfCircleCenters_h


#include "ICriterion.h"

namespace FTrack{
   
   /** Criterion: check for the change of the 2D angle
    */
   class Crit4_DistOfCircleCenters : public ICriterion{
      
      
      
   public:
      
      /**
       * @param distMax 
       */
      Crit4_DistOfCircleCenters ( float distMin , float distMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength );
      
      virtual ~Crit4_DistOfCircleCenters(){};
      
   private:
      
      float _distMax;
      float _distMin;
      
   };
   
}

#endif
