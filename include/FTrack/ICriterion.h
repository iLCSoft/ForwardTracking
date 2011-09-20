#ifndef ICriterion_h
#define ICriterion_h

#include <vector>
#include "Segment.h"
#include "lcio.h"

#include "marlin/VerbosityLevels.h"

using namespace lcio;


namespace FTrack{


   /**An Interface to model the criterion for a number of TrackerHits to be compatible for
    * some sort of hypothesis.
    * 
    *
    */   
   class ICriterion{


   public: 
      
      
      virtual bool areCompatible( Segment* parent , Segment* child ) = 0;
      
  
      std::map < std::string , float > getMapOfValues() {return _map_name_value; };
      
      virtual ~ICriterion(){};
      
     
   protected:
      
      
      std::map < std::string , float > _map_name_value;
      
      
   };
   
}


#endif




