#ifndef Crit2_PT_h
#define Crit2_PT_h

#include "ICriterion.h"


namespace FTrack{
   
   /** Criterion: ...
    */
   class Crit3_PT : public ICriterion{
      
      
      
   public:
      
      Crit3_PT ( float ptMin , float ptMax , float Bz = 3.5 );
      
      virtual bool areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength );
      
      virtual ~Crit3_PT(){};
      
      
   private:
      
      float _ptMin;
      float _ptMax;
      float _Bz;
      
      
   };
   
}













#endif

