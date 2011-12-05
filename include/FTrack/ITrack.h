#ifndef ITrack_h
#define ITrack_h

#include <vector>
#include "IHit.h"

namespace FTrack{

   
   
   class ITrack{
      
      
   public:
      
      
      virtual std::vector< IHit*> getHits() const = 0;
      
      virtual double getQI() const = 0; 
      
      virtual void fit() = 0;
      virtual double getNdf() const = 0;
      virtual double getChi2() const = 0;
      virtual double getChi2Prob() const = 0;
      
      virtual ~ITrack(){}
      
   };




}

#endif


