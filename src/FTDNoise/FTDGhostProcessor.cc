#include "FTDGhostProcessor.h"

#include <iostream>

#include <EVENT/LCCollection.h>
#include <IMPL/LCCollectionVec.h>

#include <IMPL/TrackerHitImpl.h>
#include <IMPL/TrackerHitPlaneImpl.h>

#include <UTIL/ILDConf.h>


#include <cmath>
#include <CLHEP/Random/RandFlat.h>
#include <CLHEP/Random/RandGauss.h>
#include <ctime> 
#include "streamlog/streamlog.h"

using namespace lcio ;
using namespace marlin ;
using namespace std ;

FTDGhostProcessor aFTDGhostProcessor ;


FTDGhostProcessor::FTDGhostProcessor() : Processor("FTDGhostProcessor") {
  
  // modify processor description
   _description = "FTDGhostProcessor simulates ghost hits in the FTDs" ;
  
  
  // register steering parameters: name, description, class-variable, default value
  
   
  
	
  
   // Input collection
   registerInputCollection( LCIO::TRACKERHIT,
                           "FTDCollectionName" , 
                           "Name of the FTD TrackerHit collection"  ,
                           _colNameFTD ,
                           std::string("FTDTrackerHits") ) ;
  
                           
   registerProcessorParameter( "PointResolution" ,
                              "Point Resolution"  ,
                              _pointReso ,
                              (float)0.010 ) ; 
                              
   registerProcessorParameter( "nPetalsPerDisk" ,
                               "Number of Petals for one disk" ,
                               _nPetalsPerDisk ,
                               (int) 16 ) ;

   registerProcessorParameter( "nSensorsPerPetal" ,
                               "Number of Sensors on one Petal" ,
                               _nSensorsPerPetal ,
                               (int) 2 ) ;
                               
   
  
}


void FTDGhostProcessor::init() { 

  // usually a good idea to
  printParameters() ;

  _nRun = 0 ;
  _nEvt = 0 ;
  
  //get FTD geometry info
  const gear::GearParameters& paramFTD = Global::GEAR->getGearParameters("FTD");
  
  _diskPositionZ = paramFTD.getDoubleVals( "FTDZCoordinate" ) ;
  _diskInnerRadius = paramFTD.getDoubleVals( "FTDInnerRadius" ) ;
  _diskOuterRadius = paramFTD.getDoubleVals( "FTDOuterRadius" ) ;      
         


}


void FTDGhostProcessor::processRunHeader( LCRunHeader* run) { 
  _nRun++ ;
} 

void FTDGhostProcessor::processEvent( LCEvent * evt ) { 

  LCCollection* col = 0 ;
  
  try{
    
    col = evt->getCollection( _colNameFTD ) ;
    
  }    
  catch(DataNotAvailableException &e){
    
    
    streamlog_out( WARNING ) << " FTD collection " << _colNameFTD  
                             << " not found - do nothing ! " << std::endl ;
    
    
    return ;
    
  }
  

  
//   CLHEP::HepRandom::setTheSeed( (unsigned)time(0) );
//    CLHEP::HepRandom::setTheSeed( 1344652 ); // currently used so that the results are better comparable.
 
   CellIDEncoder<TrackerHitPlaneImpl> cellid_encoder( ILDCellID0::encoder_string , col ) ;
  

   
  
  
  
  _nEvt ++ ;
}



  void FTDGhostProcessor::check( LCEvent * evt ) { 
  // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void FTDGhostProcessor::end(){ 
  
   std::cout << "FTDGhostProcessor::end()  " << name() 
            << " processed " << _nEvt << " events in " << _nRun << " runs "
            << std::endl ;
}


