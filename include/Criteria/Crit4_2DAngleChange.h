#ifndef Crit4_2DAngleChange_h
#define Crit4_2DAngleChange_h


#include "ICriterion.h"

namespace FTrack{
   
   /** Criterion: check for the change of the 2D angle
    */
   class Crit4_2DAngleChange : public ICriterion{
      
      
      
   public:
      
      /**
       * @param changeMax 
       */
      Crit4_2DAngleChange ( float changeMin , float changeMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength );
      
      virtual ~Crit4_2DAngleChange(){};
      
   private:
      
      float _changeMin;
      float _changeMax;
      
   };
   
}

#endif
