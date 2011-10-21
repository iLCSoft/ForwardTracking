#ifndef Crit2_IPCircleDist_h
#define Crit2_IPCircleDist_h

#include "ICriterion.h"


namespace FTrack{
   
   /** Criterion: ...
    */
   class Crit3_IPCircleDist : public ICriterion{
      
      
      
   public:
      
      Crit3_IPCircleDist ( float distToCircleMin , float distToCircleMax );
      
      virtual bool areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength );
      
      virtual ~Crit3_IPCircleDist(){};
      
      
   private:
      
      float _distToCircleMax;
      float _distToCircleMin;
      
      
   };
   
}













#endif

