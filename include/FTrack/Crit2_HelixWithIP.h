#ifndef Crit2_HelixWithIP_h
#define Crit2_HelixWithIP_h


#include "ICriterion.h"

namespace FTrack{

   /** Criterion: 
    */
   class Crit2_HelixWithIP : public ICriterion{



   public:
      
      Crit2_HelixWithIP ( float ratioMin , float ratioMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength );

      virtual ~Crit2_HelixWithIP(){};

    
   private:
      
      float _ratioMax;
      float _ratioMin;
      
      
      
   };

}

#endif

