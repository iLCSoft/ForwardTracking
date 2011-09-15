#ifndef Crit3_3DAngle_h
#define Crit3_3DAngle_h


#include "ICriteria.h"

namespace FTrack{

   class Crit3_3DAngle : public ICriteria{



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