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
#include <CLHEP/Random/RandGauss.h>
#include <ctime> 
#include "streamlog/streamlog.h"

using namespace lcio ;
using namespace marlin ;
using namespace std ;

FTDNoiseProcessor aFTDNoiseProcessor ;


FTDNoiseProcessor::FTDNoiseProcessor() : Processor("FTDNoiseProcessor") {
  
  // modify processor description
   _description = "FTDNoiseProcessor should create noise hits in the FTD collection" ;
  
  
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
  

   unsigned int count=0;
   
   for ( int side = -1; side <= 1; side+= 2 ){ //for both sides
   

      for ( int layer = 0; layer <= 6; layer++ ){ //over all layers


         unsigned int hitsOnThisDisk = 0; // to count the added hits for one disk

         //generate a random density that's gaussian smeared around the average.
         //And of course multiply the density with the number of integrations. (If we do this later at nHits, we will
         //for lets say 100 integrations always a multiple of 100)
         double density = CLHEP::RandGauss::shoot( _backgroundDensity[layer] * _integratedBX[ layer ] , _backgroundDensitySigma[layer] * _integratedBX[ layer ] );

         //calculate the number of hits corresponding to this density on the disk.
         //therefor: first calculate the area of the disk:
         double area = M_PI* ( _diskOuterRadius[layer]*_diskOuterRadius[layer] - _diskInnerRadius[layer]*_diskInnerRadius[layer] ) / 100.; //the division through 100 is for converting to cm

         unsigned int nHits = round( fabs( area*density ) ) ; // hit = density * area
         
         
         //So now we have the number of hits --> distribute them on the disk
         for (unsigned int iHit=0; iHit < nHits; iHit++){
            
            
            double pos[3] = { 0. , 0. , 0. } ; 

            pos[2] = _diskPositionZ[layer] * side; //the z position of the hit is exact
               
            //now we want an x and a y position.
            //For now we'll just calculate a random phi and a random R (both in the xy plane) and
            //calculate x and y from that.
            //This gives of course more hits in the region of lower R, but that's realistic anyway

            double phi = CLHEP::RandFlat::shoot ( 0. , 2*M_PI ); //angle in xy plane from 0 to 2Pi
            double R = CLHEP::RandFlat::shoot ( _diskInnerRadius[layer] , _diskOuterRadius[layer] ); // radius of the hit

            pos[0] = R* cos(phi); //x
            pos[1] = R* sin(phi); //y
            
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
            
            hitsOnThisDisk++;         

         }
         
         streamlog_out( DEBUG4 ) << "\n Added background hits on layer " << (layer+1)*side << ": " << hitsOnThisDisk;
         count += hitsOnThisDisk;

      }
   
   }
   
   streamlog_out( DEBUG4 ) << "\n\n Total added background hits on all layers: " << count << "\n";
   
   
  

  
  
  
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


