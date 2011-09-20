#include "AutCode.h"
#include "FTrackTools.h"

#include "marlin/VerbosityLevels.h"

using namespace FTrack;



AutCode::AutCode (  unsigned nLayers , unsigned nModules , unsigned nSensors ){
   
   
   _nLayers = nLayers;
   _nModules = nModules;
   _nSensors = nSensors;
   
   
}

int AutCode::getCode( int side, unsigned layer , unsigned module , unsigned sensor )throw( FTrack::OutOfRange ){
   
   //check if the values passed are okay:
   if ( ( side!= 1 )&&( side != -1 ) ){
      
    
      std::string s = "Side has to be either +1 or -1 and not " + intToString( side );
      throw OutOfRange( s );
      
   }
   
   if ( layer >= _nLayers ){
      
      std::string s = "Layer " + intToString( layer ) +" is too big, the outermost layer is layer " + intToString( _nLayers - 1 );
      throw OutOfRange( s );
      
   }
   
   if ( module >= _nModules ){
      
      std::string s = "Module " + intToString( module ) +" is too big, the highest module is module " + intToString( _nModules - 1 );
      throw OutOfRange( s );
      
   }
   
   if ( sensor >= _nSensors ){
      
      std::string s = "Sensor " + intToString( sensor ) +" is too big, the highest sensor is sensor " + intToString( _nSensors - 1 );
      throw OutOfRange( s );
      
   }   
   
   unsigned multiplicator=1;
   
   
   int code = sensor;
   multiplicator *= _nSensors; //there are nSensors possible values for sensor
   
   code += module * multiplicator;
   multiplicator *= _nModules;
   
   code += layer * multiplicator;
   multiplicator *= _nLayers;
   
   
   code += ( (side + 1 )/2 ) * multiplicator;             // (side+1) /2 gives 0 for backward (-1) and 1 for forward (+1)
   
   streamlog_out( DEBUG0 ) << "\n Code of side " << side
                           << ", layer " << layer
                           << ", module " << module
                           << ", sensor " << sensor
                           << " == " << code;
   
   return code;
   
}

int AutCode::getSide( int code ){
   
   
   int side = ( code / ( _nSensors * _nModules * _nLayers ) ) % 2; //this is an integerdivision --> we will get the floor authomatically
   
   side = side*2 - 1 ; //from 0 and 1 to  -1 and 1
   
   streamlog_out( DEBUG0 ) << "\n Code " << code << " == Side " << side;
   
   return side;
   
}

unsigned AutCode::getLayer( int code ){
   

   unsigned layer = ( code / (  _nSensors * _nModules ) ) % _nLayers; //this is an integerdivision --> we will get the floor authomatically
   
   streamlog_out( DEBUG0 ) << "\n Code " << code << " == Layer " << layer;
   
   return layer;
   
   
}

unsigned AutCode::getModule( int code ){
   

   unsigned module = ( code / ( _nSensors ) ) % _nModules; //this is an integerdivision --> we will get the floor authomatically
   
   streamlog_out( DEBUG0 ) << "\n Code " << code << " == Module " << module;
   
   return module;
   
   
   
}
unsigned AutCode::getSensor( int code ){
   
   unsigned sensor = ( code ) % _nSensors; 
   
   streamlog_out( DEBUG0 ) << "\n Code " << code << " == Sensor " << sensor;
   
   return sensor;
   
}

