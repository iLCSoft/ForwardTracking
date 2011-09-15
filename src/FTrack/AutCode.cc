#include "AutCode.h"

#include "marlin/VerbosityLevels.h"

using namespace FTrack;



AutCode::AutCode (  unsigned nLayers , unsigned nModules , unsigned nSensors ){
   
   
   _nLayers = nLayers;
   _nModules = nModules;
   _nSensors = nSensors;
   
   
}

int AutCode::getCode( int side, unsigned layer , unsigned module , unsigned sensor ){
   
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

