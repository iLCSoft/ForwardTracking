#ifndef Crit2_DeltaRho_h
#define Crit2_DeltaRho_h


#include "ICriterion.h"

namespace FTrack{

   /** Criterion: the difference of the sqrt(x^2 + y^2) of two hits.
    */
   class Crit2_DeltaRho : public ICriterion{



   public:
      
      Crit2_DeltaRho ( float deltaRhoMin , float deltaRhoMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength );

      virtual ~Crit2_DeltaRho(){};

    
   private:
      
      float _deltaRhoMax;
      float _deltaRhoMin;
      
      
      
   };

}

#endif

