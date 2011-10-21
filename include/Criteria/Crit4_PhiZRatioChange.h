#ifndef Crit4_PhiZRatioChange_h
#define Crit4_PhiZRatioChange_h


#include "ICriterion.h"

namespace FTrack{
   
   /** Criterion: check for the change of Phi / Z. (Phi = angle in the circle between two hits)
    */
   class Crit4_PhiZRatioChange : public ICriterion{
      
      
      
   public:
      
      /**
       * @param changeMax 
       */
      Crit4_PhiZRatioChange ( float changeMin , float changeMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength );
      
      virtual ~Crit4_PhiZRatioChange(){};
      
   private:
      
      float _changeMin;
      float _changeMax;
      
   };
   
}

#endif
