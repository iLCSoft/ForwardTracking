#ifndef IHitConnector_h
#define IHitConnector_h

#include <set>


namespace FTrack{
   
   
   /** Interface for determining all the sectors (of other hits) we want to connect a hit with.
    * 
    * We might for example say: a hit can only be connected to hits on the next two layers and only
    * if they are on the same petal.
    * 
    * Or we might also allow to jump directly to layer 0.
    * 
    * Or we don't want to look at layer 1 and 2 at all (for example because they are too full with noise).
    * 
    * TODO: better description, maybe rename to ISectorConnector or so?
    */
   class IHitConnector{
      
      
   public:
      
      virtual std::set <int> getTargetSectors ( int ) = 0;
      
      virtual ~IHitConnector(){};
      
   };
   
   
   
}



#endif



