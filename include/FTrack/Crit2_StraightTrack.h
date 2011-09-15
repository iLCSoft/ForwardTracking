#ifndef Crit2_StraightTrack_h
#define Crit2_StraightTrack_h


#include "ICriteria.h"

namespace FTrack{

   class Crit2_StraightTrack : public ICriteria{



   public:
      
      Crit2_StraightTrack ( double ratioMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child );


    
   private:
      
      double _ratioMax;
      
      
   };

}

#endif