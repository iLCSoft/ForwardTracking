
#ifndef AutCode_h
#define AutCode_h


#include "FTrackExceptions.h"

namespace FTrack{
   
   
   /** A class to encode and decode the place, where a hit on the FTDs is.
    * 
    * 
    * @param side: +1 for forward, -1 for backward
    * 
    * @param layer: layer of FTD: start from 1!. Layer0 == IP
    * 
    * @param module: module
    * 
    * @param sensor
    * 
    * 
    */  
   class AutCode{
     
      
   public:
      
      AutCode( unsigned nLayers , unsigned nModules , unsigned nSensors );
      
      //TODO: the numbers should be checked--> there should be an exception for wrong numbers (like layer 9, when layer 7 is max)
      
      int getCode( int side, unsigned layer , unsigned module , unsigned sensor ) throw( OutOfRange );
      
      int getSide( int code );
      unsigned getLayer( int code );
      unsigned getModule( int code ); 
      unsigned getSensor( int code );

      unsigned getNLayers(){ return _nLayers; };
      unsigned getNModules(){ return _nModules; };
      unsigned getNSensors(){ return _nSensors; };
      
   private:
      
      
      unsigned _nLayers;
      unsigned _nModules;
      unsigned _nSensors;
      
      
      
   };
   
   
}


#endif

