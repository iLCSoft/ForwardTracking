#ifndef IHitConnector_h
#define IHitConnector_h

#include <set>
#include "UTIL/ILDConf.h"

namespace FTrack{
   
   
   /** Interface for determining all the Codes (of toher hits) we want to connect a hit with.
    * 
    * We might for example say: a hit can only be connected to hits on the next two layers and only
    * if they are on the same petal.
    * 
    * Or we might also allow to jump directly to layer 0.
    * 
    * Or we don't want to look at layer 1 and 2 at all (for example because they are too full with noise).
    * 
    * 
    * I suppose that the hits have some sort of code. This can be anything: simply the layers or the CellID0
    * ( or CellID1 ) or an own code (like autCode provides).
    * 
    *
    * This interface is for determining what the codes (layers, CellID0 ...) are a hit with an own certain code
    * may connect to.
    *
    * 
    */
   class IHitConnector{
      
      
   public:
      
      virtual std::set <int> getTargetCode ( int ) = 0;
      

   };
   
   
   
}



#endif


