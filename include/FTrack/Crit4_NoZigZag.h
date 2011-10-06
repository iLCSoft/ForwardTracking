#ifndef Crit4_NoZigZag_h
#define Crit4_NoZigZag_h


#include "ICriterion.h"

namespace FTrack{
   
   /** Criterion: forbids zig zagging
    */
   class Crit4_NoZigZag : public ICriterion{
      
      
      
   public:
      
      /**
       * @param prodMin the minimum product of the two angles in the range from -180° to 180° TODO: explain better
       */
      Crit4_NoZigZag ( double prodMin );
      
      virtual bool areCompatible( Segment* parent , Segment* child );
      
      virtual ~Crit4_NoZigZag(){};
      
   private:
      
      double _prodMin;
      
   };
   
}

#endif
