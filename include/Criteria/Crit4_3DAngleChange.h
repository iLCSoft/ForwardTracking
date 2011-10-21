#ifndef Crit4_3DAngleChange_h
#define Crit4_3DAngleChange_h


#include "ICriterion.h"

namespace FTrack{
   
   /** Criterion: check for the change of the 2D angle
    */
   class Crit4_3DAngleChange : public ICriterion{
      
      
      
   public:
      
      /**
       * @param changeMax 
       */
      Crit4_3DAngleChange ( float changeMin , float changeMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength );
      
      virtual ~Crit4_3DAngleChange(){};
      
   private:
      
      float _changeMin;
      float _changeMax;
      
   };
   
}

#endif
