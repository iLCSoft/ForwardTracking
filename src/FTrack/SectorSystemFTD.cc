#include "SectorSystemFTD.h"
#include "FTrackTools.h"

using namespace FTrack;

SectorSystemFTD::SectorSystemFTD( unsigned nLayers , unsigned nModules , unsigned nSensors ):
   

_nModules( nModules ),
_nSensors( nSensors ){
   
   _nLayers = nLayers;
   _sectorMax = 2*nLayers*nModules*nSensors - 1;
   
}
   


int SectorSystemFTD::getSector( int side, unsigned layer , unsigned module , unsigned sensor )const throw( FTrack::OutOfRange ){
   
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
   
   
   int sector = sensor;
   multiplicator *= _nSensors; //there are nSensors possible values for sensor
   
   sector += module * multiplicator;
   multiplicator *= _nModules;
   
   sector += layer * multiplicator;
   multiplicator *= _nLayers;
   
   
   sector += ( (side + 1 )/2 ) * multiplicator;             // (side+1) /2 gives 0 for backward (-1) and 1 for forward (+1)
   
   streamlog_out( DEBUG0 ) << "\n Sector of side " << side
   << ", layer " << layer
   << ", module " << module
   << ", sensor " << sensor
   << " == " << sector;
   
   return sector;
   
}






int SectorSystemFTD::getSide( int sector ) const throw ( OutOfRange ){
   
   checkSectorIsInRange( sector );
   
   
   
   int side = ( sector / ( _nSensors * _nModules * _nLayers ) ) % 2; //this is an integerdivision --> we will get the floor authomatically
   
   side = side*2 - 1 ; //from 0 and 1 to  -1 and 1
   
   streamlog_out( DEBUG0 ) << "\n Sector " << sector << " == Side " << side;
   
   return side;
   
}

unsigned SectorSystemFTD::getLayer( int sector ) const throw ( OutOfRange ){
   
   checkSectorIsInRange( sector );
   
   unsigned layer = ( sector / (  _nSensors * _nModules ) ) % _nLayers; //this is an integerdivision --> we will get the floor authomatically
   
   streamlog_out( DEBUG0 ) << "\n Sector " << sector << " == Layer " << layer;
   
   return layer;
   
   
}

unsigned SectorSystemFTD::getModule( int sector ) const throw ( OutOfRange ){
   
   
   checkSectorIsInRange( sector );
   
   unsigned module = ( sector / ( _nSensors ) ) % _nModules; //this is an integerdivision --> we will get the floor authomatically
   
   streamlog_out( DEBUG0 ) << "\n Sector " << sector << " == Module " << module;
   
   return module;
   
   
   
}
unsigned SectorSystemFTD::getSensor( int sector ) const throw ( OutOfRange ){
   
   
   checkSectorIsInRange( sector );
   
   unsigned sensor = ( sector ) % _nSensors; 
   
   streamlog_out( DEBUG0 ) << "\n Sector " << sector << " == Sensor " << sensor;
   
   return sensor;
   
}


void SectorSystemFTD::checkSectorIsInRange( int sector ) const throw ( OutOfRange ){


   if ( sector > _sectorMax ){
      
      std::string s = "SectorSystemFTD:\n Sector " + intToString( sector ) +" is too big, the highest possible number for a sector in this configuration of FTDSegRepresentation is"
      + intToString( _sectorMax ) + ".\nThe configuration is: nLayers = " + intToString( _nLayers ) +
      ", nModules = " + intToString( _nModules ) + 
      ", nSensors = " + intToString( _nSensors ) +
      "\n With 2 sides (forward and backward) this gives sectors from 0 to 2*" 
      + intToString( _nLayers ) + "*"  
      + intToString( _nModules ) + "*"
      + intToString( _nSensors ) + " -1 = " + intToString( 2*_nLayers*_nModules*_nSensors -1 );
      throw OutOfRange( s );
      
   }  

}


