#ifndef Crit2_RZRatio_h
#define Crit2_RZRatio_h


#include "ICriterion.h"

namespace FTrack{

   /** Cirterion: distance of two hits divided by their z-distance
    */
   class Crit2_RZRatio : public ICriterion{



   public:
      
      Crit2_RZRatio ( float ratioMin, float ratioMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength );

      virtual ~Crit2_RZRatio(){};
    
   private:
      
      float _ratioMax;
      float _ratioMin;
      
      
   };

}

#endif

