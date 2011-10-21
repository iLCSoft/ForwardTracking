#ifndef Crit2_ChangeRZRatio_h
#define Crit2_ChangeRZRatio_h

#include "ICriterion.h"


namespace FTrack{
   
   /** Criterion: ...
    */
   class Crit3_ChangeRZRatio : public ICriterion{
      
      
      
   public:
      
      Crit3_ChangeRZRatio ( float minChange , float maxChange );
      
      virtual bool areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength );
      
      virtual ~Crit3_ChangeRZRatio(){};
      
      
   private:
      
      float _ratioChangeMaxSquared;
      float _ratioChangeMinSquared;

      
      
   };
   
}













#endif

