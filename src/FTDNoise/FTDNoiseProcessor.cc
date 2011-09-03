#include "FTDNoiseProcessor.h"

#include <iostream>

#include <EVENT/LCCollection.h>
#include <IMPL/LCCollectionVec.h>

#include <IMPL/TrackerHitImpl.h>




#include <cmath>
#include <math.h>
#include <CLHEP/Random/RandFlat.h>
#include <ctime> 


using namespace lcio ;
using namespace marlin ;
using namespace std ;

FTDNoiseProcessor aFTDNoiseProcessor ;


FTDNoiseProcessor::FTDNoiseProcessor() : Processor("FTDNoiseProcessor") {
  
  // modify processor description
   _description = "FTDNoiseProcessor should create noise hits in the FTD collection" ;
  
  
  // register steering parameters: name, description, class-variable, default value
  
   
  
  registerProcessorParameter( "nNoiseHits" ,
                              "Number of noise hits we want to add to the FTD"  ,
                              _nNoiseHits ,
                              0 ) ;
	
  
  // Input collection
  registerInputCollection( LCIO::TRACKERHIT,
                           "FTDCollectionName" , 
                           "Name of the FTD TrackerHit collection"  ,
                           _colNameFTD ,
                           std::string("FTDTrackerHits") ) ;
  
  

  
}


void FTDNoiseProcessor::init() { 

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


void FTDNoiseProcessor::processRunHeader( LCRunHeader* run) { 
  _nRun++ ;
} 

void FTDNoiseProcessor::processEvent( LCEvent * evt ) { 

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
  CLHEP::HepRandom::setTheSeed( 1344652 ); // currently used so that the results are better comparable.
 

  for (int i = 0; i< _nNoiseHits; i++){
  
     
     TrackerHitImpl *hit = new TrackerHitImpl() ;
      
     
     int layer = CLHEP::RandFlat::shootInt(1,8);      //The layer: 1-7
     bool isForward = CLHEP::RandFlat::shootBit();    //forwar or backward
     
     double pos[3] = { 0. , 0. , 0. } ;

     pos[2] = _diskPositionZ[layer-1]; //the z position of the hit is the random layer
     if (!isForward) pos[2] *= -1.;
     
     //now we want an x and a y position.
     //For now we'll just calculate a random phi and a random R (both in the xy plane) and
     //calculate x and y from that.
     
     double phi = CLHEP::RandFlat::shoot ( 0. , 2*M_PI ); //angle in xy plane from 0 to 2Pi
     double R = CLHEP::RandFlat::shoot ( _diskInnerRadius[layer-1] , _diskOuterRadius[layer-1] );
     
     pos[0] = R* cos(phi);
     pos[1] = R* sin(phi);
     
     hit->setPosition( pos ) ;
          

     
//      std::cout<< std::endl<<layer <<" : " <<isForward <<" R,phi= " << R << "," << phi << "  pos= " << pos[0] << "," << pos[1] <<"," << pos[2];
     
     hit->setType (200 + layer);
     
     col->addElement( hit ) ; 
     
  
  }
  
  
  
  _nEvt ++ ;
}



  void FTDNoiseProcessor::check( LCEvent * evt ) { 
  // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void FTDNoiseProcessor::end(){ 
  
   std::cout << "FTDNoiseProcessor::end()  " << name() 
            << " processed " << _nEvt << " events in " << _nRun << " runs "
            << std::endl ;
}


