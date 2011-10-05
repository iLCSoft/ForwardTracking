#ifndef Crit2_PTMin_h
#define Crit2_PTMin_h

#include "ICriterion.h"


namespace FTrack{
   
   /** Criterion: ...
    */
   class Crit3_PTMin : public ICriterion{
      
      
      
   public:
      
      Crit3_PTMin ( double ptMin , double _Bz = 3.5 );
      
      virtual bool areCompatible( Segment* parent , Segment* child );
      
      virtual ~Crit3_PTMin(){};
      
      
   private:
      
      double _ptMin;
      double _Bz;
      
      
   };
   
}













#endif

