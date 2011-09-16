#ifndef ICriteria_h
#define ICriteria_h

#include <vector>
#include "Segment.h"
#include "lcio.h"

using namespace lcio;


namespace FTrack{


   /**An Interface to model the criteria for a number of TrackerHits to be compatible for
    * some sort of hypothesis.
    * 
    *
    */   
   class ICriteria{


   public: 
      
      
      virtual bool areCompatible( Segment* parent , Segment* child ) = 0;
      
  
      std::map < std::string , float > getMapOfValues() {return _map_name_value; };
      
      
     
   protected:
      
      
      std::map < std::string , float > _map_name_value;
      
      
   };
   
}


#endif


