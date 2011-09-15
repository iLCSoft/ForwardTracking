#ifndef FTDRepresentation_h
#define FTDRepresentation_h


#include "AutHit.h"
#include <map>
#include <set>
#include <utility>
#include "AutCode.h"



namespace FTrack{


   /**A class representing the FTD.\ In it the hits on the FTD can be stored
    * and accessed more easy.
    */
   class FTDRepresentation {
      
   public:
      
      /**
       * @param nLayers: number of layers
       * 
       * @param nModules: the number of modules per layer
       * 
       * @param nSensors: number of sensors per module
       */
      FTDRepresentation( AutCode* autCode );
      
      /**Adds a AutHit to the object.
       * 
       */
      void addHit(  AutHit* hit );
      
      /** Returns all Hits from one specific sensor.
       */
      std::vector <  AutHit* > getHitsFromSensor( int side , unsigned int layer , unsigned int module , unsigned int sensor );
      std::vector <  AutHit* > getHitsFromModule( int side , unsigned int layer , unsigned int module );
      std::vector <  AutHit* > getHitsFromLayer ( int side , unsigned int layer );
      std::vector <  AutHit* > getHitsFromSide  ( int side );
      std::vector <  AutHit* > getAllHits();
      
      unsigned int getNLayers()   { return _nLayers;  };
      unsigned int getNModules()  { return _nModules; };
      unsigned int getNSensors()  { return _nSensors; };
      
      
      AutCode* getAutCode() { return _autCode; };

      
      std::vector < AutHit* > getHitsWithCode ( int code ) { return _map_code_hits[ code ]; };
      std::set <int> getCodes();
      
   private:
      
      /** A map containing the code as the key and as value a vector of all AutHits* with this code
       */
      std::map < int , std::vector < AutHit*> > _map_code_hits;


      //Do we want those variables? Because the info is contained in _autCode as well.
      unsigned _nLayers;
      unsigned _nModules;
      unsigned _nSensors;
      
      AutCode* _autCode;
      



   };

}


#endif