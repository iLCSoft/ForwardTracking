#ifndef CritRZRatio_h
#define CritRZRatio_h


#include "ICriterion.h"

namespace FTrack{

   /** Cirterion: distance of two hits divided by their z-distance
    */
   class CritRZRatio : public ICriterion{



   public:
      
      CritRZRatio ( double ratioMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child );

      virtual ~CritRZRatio(){};
    
   private:
      
      double _ratioMax;
      
      
   };

}

#endif

