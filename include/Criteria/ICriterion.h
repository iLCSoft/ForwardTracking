#ifndef ICriterion_h
#define ICriterion_h

#include <vector>
#include <map>
#include <string>

#include "Segment.h"
#include "FTrackExceptions.h"



namespace FTrack{


   /**An Interface to model the criterion for a number of TrackerHits to be compatible for
    * some sort of hypothesis.
    * 
    *
    */   
   class ICriterion{


   public: 
      
      
      virtual bool areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength ) = 0;
      
  
      std::map < std::string , float > getMapOfValues() {return _map_name_value; };
      
            
      virtual ~ICriterion(){};
      
      /** sets, whether the values calculated shall be saved in a map
       */
      void setSaveValues( bool saveValues ){ _saveValues = saveValues;}
      
      
      
      std::string getName(){return _name;}
      std::string getType(){return _type;}
     
   protected:
      
      
      std::map < std::string , float > _map_name_value;
      
      bool _saveValues;
      
      std::string _name;
      std::string _type;
      
   };
   
}


#endif




