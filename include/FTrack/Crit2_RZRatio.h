#ifndef Crit2_RZRatio_h
#define Crit2_RZRatio_h


#include "ICriterion.h"

namespace FTrack{

   /** Cirterion: distance of two hits divided by their z-distance
    */
   class Crit2_RZRatio : public ICriterion{



   public:
      
      Crit2_RZRatio ( double ratioMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child );

      virtual ~Crit2_RZRatio(){};
    
   private:
      
      double _ratioMax;
      
      
   };

}

#endif

