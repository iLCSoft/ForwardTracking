#ifndef FTDSegRepresentation_h
#define FTDSegRepresentation_h

#include "AutHit.h"
#include <map>
#include <utility>
#include "AutCode.h"
#include <set>

#include "Segment.h"



namespace FTrack{

   //TODO: is this the best way to combine 1-segments and the CellID0? Should this be somehow combined with FTDRepresentation,
   // or should even be one replaced with the other? 

   /**A class representing the FTD.\ In it the 1-segments of the FTD can be stored
    * and accessed more easy. It is very similar to FTDRepresentation and be they can be combined in a more efficient way.
    */
   class FTDSegRepresentation {
      
   public:
      
      /**
       * @param autCode the autCode that will be used to encode and decode the autHits.
       */
      FTDSegRepresentation( AutCode* autCode );
      
      /**Adds a segment to the object.
       * 
       */
      void addSegment( Segment* seg , int code  );
      
      /** Returns all Segments from one specific sensor.
       */
      std::vector < Segment* > getSegsFromSensor( int side , unsigned int layer , unsigned int module , unsigned int sensor );
      std::vector < Segment* > getSegsFromModule( int side , unsigned int layer , unsigned int module );
      std::vector < Segment* > getSegsFromLayer ( int side , unsigned int layer );
      std::vector < Segment* > getSegsFromSide  ( int side );
      std::vector < Segment* > getAllSegs();
      
      std::vector < Segment* > getSegsWithCode ( int code ) { return _map_code_segs[ code ]; };
      
      /**returns a set of all the codes that are used.
       */
      std::set < int > getCodes();
      
      unsigned int getNLayers()   { return _nLayers;  };
      unsigned int getNModules()  { return _nModules; };
      unsigned int getNSensors()  { return _nSensors; };
      
      AutCode* getAutCode() { return _autCode; };
      

      
   private:
      
      /** A map containing the code as the key and as value a vector of all Segments* with this code
       */
      std::map < int , std::vector < Segment*> > _map_code_segs;
      
      
      
      unsigned _nLayers;
      unsigned _nModules;
      unsigned _nSensors;

      
      AutCode* _autCode;

   };

}


#endif