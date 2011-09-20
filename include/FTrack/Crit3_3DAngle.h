#ifndef Crit3_3DAngle_h
#define Crit3_3DAngle_h


#include "ICriterion.h"

namespace FTrack{

   /** Criterion: the angle between two 2-segments
    */
   class Crit3_3DAngle : public ICriterion{



   public:
      
      /**
       * @param angleMax the maximum angle between 2 2-segments in grad
       */
      Crit3_3DAngle ( double angleMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child );


    
   private:
      
      double _cosAngleMin;
      
   };

}

#endif

