#ifndef Crit4_RChange_h
#define Crit4_RChange_h


#include "ICriterion.h"

namespace FTrack{
   
   /** Criterion: check for the change of the 2D angle
    */
   class Crit4_RChange : public ICriterion{
      
      
      
   public:
      
      /**
       * @param changeMax 
       */
      Crit4_RChange ( double changeMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child );
      
      virtual ~Crit4_RChange(){};
      
   private:
      
      double _changeMax;
      
   };
   
}

#endif
