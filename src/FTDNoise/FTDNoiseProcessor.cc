#include "FTDNoiseProcessor.h"

#include <iostream>

#include <EVENT/LCCollection.h>
#include <IMPL/LCCollectionVec.h>

#include <IMPL/TrackerHitImpl.h>
#include <IMPL/TrackerHitPlaneImpl.h>

#include <UTIL/ILDConf.h>


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
  
                           
  registerProcessorParameter( "PointResolution" ,
                              "Point Resolution"  ,
                              _pointReso ,
                              (float)0.010 ) ;                           
  

  
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
 
   CellIDEncoder<TrackerHitPlaneImpl> cellid_encoder( ILDCellID0::encoder_string , col ) ;
  
   for (int i = 0; i< _nNoiseHits; i++){
  
     
      TrackerHitImpl *hit = new TrackerHitImpl() ;


      int layer = CLHEP::RandFlat::shootInt(7);      //The layer: 0-6
      bool isForward = CLHEP::RandFlat::shootBit();    //forward or backward
      int side = 1;
      if (!isForward) side = -1;

      double pos[3] = { 0. , 0. , 0. } ;

      pos[2] = _diskPositionZ[layer] * side; //the z position of the hit is the random layer
      

      //now we want an x and a y position.
      //For now we'll just calculate a random phi and a random R (both in the xy plane) and
      //calculate x and y from that.

      double phi = CLHEP::RandFlat::shoot ( 0. , 2*M_PI ); //angle in xy plane from 0 to 2Pi
      double R = CLHEP::RandFlat::shoot ( _diskInnerRadius[layer-1] , _diskOuterRadius[layer-1] );

      pos[0] = R* cos(phi);
      pos[1] = R* sin(phi);


      TrackerHitPlaneImpl* trkHit = new TrackerHitPlaneImpl ;        


      
      
      trkHit->setType( 201+layer);  // needed for FullLDCTracking et al.

      cellid_encoder[ ILDCellID0::subdet ] = ILDDetID::FTD  ;
      cellid_encoder[ ILDCellID0::side   ] = side ;
      cellid_encoder[ ILDCellID0::layer  ] = layer ;
      cellid_encoder[ ILDCellID0::module ] = 0 ;
      cellid_encoder[ ILDCellID0::sensor ] = 0 ;
      
      cellid_encoder.setCellID( trkHit ) ;

      trkHit->setPosition(  pos  ) ;

      float u_direction[2] ; // x
      u_direction[0] = 0.0 ; 
      u_direction[1] = M_PI/2.0 ;
      
      float v_direction[2] ; // y
      v_direction[0] = M_PI/2.0 ;
      v_direction[1] = M_PI/2.0 ;
      
      trkHit->setU( u_direction ) ;
      trkHit->setV( v_direction ) ;
      
      trkHit->setdU( _pointReso ) ;
      trkHit->setdV( _pointReso ) ;
               
      trkHit->setEDep( 0. ) ;



      col->addElement( trkHit ) ; 

  
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


