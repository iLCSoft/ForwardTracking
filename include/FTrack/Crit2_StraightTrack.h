#ifndef Crit2_StraightTrack_h
#define Crit2_StraightTrack_h


#include "ICriterion.h"

namespace FTrack{

   /** Criterion: for straight tracks, if line between two hits point towards IP
    */
   class Crit2_StraightTrack : public ICriterion{



   public:
      
      Crit2_StraightTrack ( double ratioMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child );


    
   private:
      
      double _ratioMax;
      
      
   };

}

#endif