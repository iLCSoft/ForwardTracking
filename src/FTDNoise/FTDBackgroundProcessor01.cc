#include "FTDBackgroundProcessor01.h"

#include <iostream>

#include <EVENT/LCCollection.h>
#include <IMPL/LCCollectionVec.h>

#include <IMPL/TrackerHitImpl.h>
#include <IMPL/TrackerHitPlaneImpl.h>

#include <UTIL/ILDConf.h>

#include <marlin/Global.h>
#include <gear/GEAR.h>
#include <gear/GearParameters.h>
#include "gear/FTDParameters.h"
#include "gear/FTDLayerLayout.h"

#include <cmath>
#include <math.h>
#include <CLHEP/Random/RandFlat.h>
#include <CLHEP/Random/RandGauss.h>
#include <ctime> 
#include "streamlog/streamlog.h"

using namespace lcio ;
using namespace marlin ;
using namespace std ;

FTDBackgroundProcessor01 aFTDBackgroundProcessor01 ;


FTDBackgroundProcessor01::FTDBackgroundProcessor01() : Processor("FTDBackgroundProcessor01") {
  
  // modify processor description
   _description = "FTDBackgroundProcessor01 should create noise hits in the FTD collection" ;
  
  
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
   
   registerProcessorParameter( "DensityRegulator",
                               "Regulates all densities. So this can be used to dim or amplify all the background. 1 means no change at all",
                               _densityRegulator,
                               float( 1. ) );
   
   
   std::vector < float > defaultBgDensity;
   
   defaultBgDensity.push_back ( 0.013 );
   defaultBgDensity.push_back ( 0.008 );
   defaultBgDensity.push_back ( 0.002 );
   defaultBgDensity.push_back ( 0.002 );
   defaultBgDensity.push_back ( 0.001 );
   defaultBgDensity.push_back ( 0.001 );
   defaultBgDensity.push_back ( 0.001 );
   
   registerProcessorParameter( "BackgroundHitDensity" ,
                               "Density of background hits in hits / cm^2 / BX" ,
                               _backgroundDensity ,
                               defaultBgDensity );
   
   
   std::vector < float > defaultBgDensitySigma;
   
   defaultBgDensitySigma.push_back ( 0.005 );
   defaultBgDensitySigma.push_back ( 0.003 );
   defaultBgDensitySigma.push_back ( 0.001 );
   defaultBgDensitySigma.push_back ( 0.001 );
   defaultBgDensitySigma.push_back ( 0.001 );
   defaultBgDensitySigma.push_back ( 0.001 );
   defaultBgDensitySigma.push_back ( 0.001 );
   
   registerProcessorParameter( "BackgroundHitDensitySigma" ,
                               "Standard deviation of density of background hits in hits / cm^2 / BX" ,
                               _backgroundDensitySigma ,
                               defaultBgDensitySigma );
  
   
   std::vector < int > defaultIntegratedBX;
   
   defaultIntegratedBX.push_back ( 100 );
   defaultIntegratedBX.push_back ( 100 );
   defaultIntegratedBX.push_back ( 1 );
   defaultIntegratedBX.push_back ( 1 );
   defaultIntegratedBX.push_back ( 1 );
   defaultIntegratedBX.push_back ( 1 );
   defaultIntegratedBX.push_back ( 1 );
   
   registerProcessorParameter( "IntegratedBX" ,
                               "Number of bunchcrossings that are integrated" ,
                               _integratedBX ,
                               defaultIntegratedBX );

  
}


void FTDBackgroundProcessor01::init() { 

  // usually a good idea to
  printParameters() ;

  _nRun = 0 ;
  _nEvt = 0 ;
  
 
  


}


void FTDBackgroundProcessor01::processRunHeader( LCRunHeader* run) { 
  _nRun++ ;
} 

void FTDBackgroundProcessor01::processEvent( LCEvent * evt ) { 

  LCCollection* col = 0 ;
  
  try{
    
    col = evt->getCollection( _colNameFTD ) ;
    
  }    
  catch(DataNotAvailableException &e){
    
    
    streamlog_out( WARNING ) << " FTD collection " << _colNameFTD  
                             << " not found - do nothing ! " << std::endl ;
    
    
    return ;
    
  }
  
  //get FTD geometry info
  const gear::FTDParameters& ftdParams = Global::GEAR->getFTDParameters() ;
  const gear::FTDLayerLayout& ftdLayers = ftdParams.getFTDLayerLayout() ;
  
  _nLayers = ftdLayers.getNLayers(); 
  int sensor = 1;

  
//   CLHEP::HepRandom::setTheSeed( (unsigned)time(0) );
   CLHEP::HepRandom::setTheSeed( 1344652 ); // currently used so that the results are better comparable.
 
   CellIDEncoder<TrackerHitPlaneImpl> cellid_encoder( ILDCellID0::encoder_string , col ) ;
  

   unsigned int count=0;
   
   for ( int side = -1; side <= 1; side+= 2 ){ //for both sides
   

      for ( unsigned layer = 0; layer < _nLayers; layer++ ){ //over all layers

         unsigned hitsOnThisDisk=0;

         unsigned nModules = ftdLayers.getNPetals(layer);
         
         // area of one petal
         double lMin = ftdLayers.getSensitiveLengthMin(layer);
         double lMax = ftdLayers.getSensitiveLengthMax(layer);
         double width = ftdLayers.getSensitiveWidth(layer);
         double rMin = ftdLayers.getSensitiveRinner(layer);
         double area = ( lMin + lMax ) / 2. * width  / 100.; //the division through 100 is for converting to cm

         for (unsigned petal = 0; petal < nModules; petal++ ){ //over all petals

            
            //generate a random density that's gaussian smeared around the average.
            //And of course multiply the density with the number of integrations. (If we do this later at nHits, we will
            //for lets say 100 integrations always a multiple of 100)
            double density = _densityRegulator * CLHEP::RandGauss::shoot( _backgroundDensity[layer] * _integratedBX[ layer ] , _backgroundDensitySigma[layer] * _integratedBX[ layer ] );

            //calculate the number of hits corresponding to the density on the disk.
            unsigned nHits = unsigned  ( round( fabs( area*density ) ) ); // hit = density * area
            
            
            //So now we have the number of hits --> distribute them on the petal
            for (unsigned int iHit=0; iHit < nHits; iHit++){
               
               
               double pos[3] = { 0. , 0. , 0. } ; 

               pos[2] = side * ftdLayers.getSensitiveZposition( layer, petal, 1 ); //the z position of the hit is exact, sensor is taken to be 1
                  
               // now we want an x and a y position.
               // As hits are not evenly distributed in xy, there will be more hits near the beam,
               // this simple approach might be alright:
               // --> make a trapezoid with same shape centered around the x axis and with the bottom sitting at x = 0,
               // --> get a evenly distributed random number for x between the bottom and the top
               // --> calculate the y - width at this x
               // --> get a evenly distributed random number for y between left side and right side
               // --> transfom this coordinates to those of the actual trapezoid
               
             
               double x = CLHEP::RandFlat::shoot ( 0. ,width ); 
               double yWidth = lMin + (lMax-lMin) * ( x / width );
               
               double y = CLHEP::RandFlat::shoot ( -1.*yWidth/2. , yWidth/2. ); 
               
               // now transform to actual trapezoid
               x += rMin;
               
               // now it has the right distance from 0. But it still needs rotation
               double R = sqrt( x*x + y*y );
               double phi = atan2( y, x );
               
               phi += ftdLayers.getPhiPetalCd( layer, petal);
               
               
               pos[0] = R* cos(phi); //x
               pos[1] = R* sin(phi); //y
               
               streamlog_out( DEBUG1 ) << "\nAdded hit at x = " << pos[0] << ", y = " << pos[1] << ", z = " << pos[2]
                                       << "\n  side = " << side  << ", layer = " << layer
                                       <<", module = " << petal << ", sensor = " << sensor;
               
               TrackerHitPlaneImpl* trkHit = new TrackerHitPlaneImpl ;        

               
               trkHit->setType( 201+layer);  // needed for FullLDCTracking et al.

               
               cellid_encoder[ ILDCellID0::subdet ] = ILDDetID::FTD  ;
               cellid_encoder[ ILDCellID0::side   ] = side ;
               cellid_encoder[ ILDCellID0::layer  ] = layer ;
               cellid_encoder[ ILDCellID0::module ] = petal ;
               cellid_encoder[ ILDCellID0::sensor ] = sensor ;
               
               
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
               
               hitsOnThisDisk++;         

            }
            
         }
         
         streamlog_out( DEBUG4 ) << "\n Added background hits on layer " << int(layer+1)*side << ": " << hitsOnThisDisk;
         count += hitsOnThisDisk;
         
      }
   
   }
   
   streamlog_out( DEBUG4 ) << "\n\n Total added background hits on all layers: " << count << "\n";
   
   
  

  
  
  
  _nEvt ++ ;
}



  void FTDBackgroundProcessor01::check( LCEvent * evt ) { 
  // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void FTDBackgroundProcessor01::end(){ 
  
   std::cout << "FTDBackgroundProcessor01::end()  " << name() 
            << " processed " << _nEvt << " events in " << _nRun << " runs "
            << std::endl ;
}




