#ifndef IHitConnector_h
#define IHitConnector_h

#include <set>
#include "UTIL/ILDConf.h"

namespace FTrack{
   
   
   /** Interface for determining all the Codes we want to connect a hit with.
    * 
    * We might for example say: a hit can only be connected to hits in the next two layers and only
    * if they are on the same petal.
    * 
    * We might skip layers etc.
    * 
    * 
    * The code that is used to encode and decode might be any arbitrary code. Of course this should then be
    * used throughout the rest of the code as well.
    */
   class IHitConnector{
      
      
   public:
      
      virtual std::set <int> getTargetCode ( int ) = 0;
      

   };
   
   
   
}



#endif


