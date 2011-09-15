#ifndef CritRZRatio_h
#define CritRZRatio_h


#include "ICriteria.h"

namespace FTrack{

   class CritRZRatio : public ICriteria{



   public:
      
      CritRZRatio ( double ratioMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child );


    
   private:
      
      double _ratioMax;
      
      
   };

}

#endif