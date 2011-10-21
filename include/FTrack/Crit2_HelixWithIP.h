#ifndef Crit2_HelixWithIP_h
#define Crit2_HelixWithIP_h


#include "ICriterion.h"

namespace FTrack{

   /** Criterion: 
    */
   class Crit2_HelixWithIP : public ICriterion{



   public:
      
      Crit2_HelixWithIP ( double ratioMin , double ratioMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength );

      virtual ~Crit2_HelixWithIP(){};

    
   private:
      
      double _ratioMax;
      double _ratioMin;
      
      
      
   };

}

#endif

