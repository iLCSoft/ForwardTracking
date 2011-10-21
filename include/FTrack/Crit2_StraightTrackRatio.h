#ifndef Crit2_StraightTrackRatio_h
#define Crit2_StraightTrackRatio_h


#include "ICriterion.h"

namespace FTrack{

   /** Criterion: for straight tracks, if line between two hits point towards IP
    */
   class Crit2_StraightTrackRatio : public ICriterion{



   public:
      
      Crit2_StraightTrackRatio ( float ratioMin, float ratioMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength );

      virtual ~Crit2_StraightTrackRatio(){};

    
   private:
      
      float _ratioMax;
      float _ratioMin;
      
      
      
   };

}

#endif

