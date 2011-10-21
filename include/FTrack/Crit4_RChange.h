#ifndef Crit4_RChange_h
#define Crit4_RChange_h


#include "ICriterion.h"

namespace FTrack{
   
   /** Criterion: check for the change of the 2D angle
    */
   class Crit4_RChange : public ICriterion{
      
      
      
   public:
      
      /**
       * @param changeMax 
       */
      Crit4_RChange ( float changeMin , float changeMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength );
      
      virtual ~Crit4_RChange(){};
      
   private:
      
      float _changeMin;
      float _changeMax;
      
   };
   
}

#endif
