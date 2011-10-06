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
      Crit4_2DAngleChange ( double changeMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child );
      
      virtual ~Crit4_2DAngleChange(){};
      
   private:
      
      double _changeMax;
      
   };
   
}

#endif
