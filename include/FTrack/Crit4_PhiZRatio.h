#ifndef Crit4_PhiZRatio_h
#define Crit4_PhiZRatio_h


#include "ICriterion.h"

namespace FTrack{
   
   /** Criterion: check for the change of the 2D angle
    */
   class Crit4_PhiZRatio : public ICriterion{
      
      
      
   public:
      
      /**
       * @param changeMax 
       */
      Crit4_PhiZRatio ( double changeMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child );
      
      virtual ~Crit4_PhiZRatio(){};
      
   private:
      
      double _changeMax;
      
   };
   
}

#endif
