#include "FTDBackgroundProcessor.h"

#include <cmath>

#include <CLHEP/Random/RandFlat.h>
#include <CLHEP/Random/RandGauss.h>


#include "EVENT/LCCollection.h"
#include "IMPL/LCCollectionVec.h"
#include "IMPL/TrackerHitPlaneImpl.h"
#include "UTIL/ILDConf.h"
#include "marlin/Global.h"
#include "marlin/ProcessorEventSeeder.h"
#include "marlin/VerbosityLevels.h"

#include "gear/GEAR.h"
#include "gear/GearParameters.h"
#include "gear/FTDParameters.h"
#include "gear/FTDLayerLayout.h"

#include "MarlinTrk/Factory.h"
#include "gear/gearsurf/MeasurementSurfaceStore.h"
#include "gear/gearsurf/MeasurementSurface.h"
#include "gear/gearsurf/ICoordinateSystem.h"
#include "gear/gearsurf/CartesianCoordinateSystem.h"



using namespace lcio ;
using namespace marlin ;
using namespace std ;

FTDBackgroundProcessor aFTDBackgroundProcessor ;


FTDBackgroundProcessor::FTDBackgroundProcessor() : Processor("FTDBackgroundProcessor") {
  
  // modify processor description
   _description = "FTDBackgroundProcessor creates salt and pepper background hits on the FTD" ;
  
 
  
   // Input collection
   registerInputCollection( LCIO::TRACKERHIT,
                           "FTDPixelTrackerHitCollectionName" , 
                           "Name of the FTD Pixel TrackerHit collection"  ,
                           _colNameFTDPixelTrackerHit ,
                            std::string("FTDPixelTrackerHits") ) ;
   
   // Input collection
   registerInputCollection( LCIO::TRACKERHIT,
                           "FTDStripTrackerHitCollectionName" , 
                           "Name of the FTD Strip TrackerHit collection"  ,
                           _colNameFTDStripTrackerHit ,
                            std::string("FTDStripTrackerHits") ) ;
  
   registerProcessorParameter( "ResolutionU" ,
                              "resolution in direction of u"  ,
                              _resU ,
                              float(0.0040)) ;
   
   registerProcessorParameter( "ResolutionV" , 
                              "resolution in direction of v" ,
                              _resV ,
                              float(0.0040));
   
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


void FTDBackgroundProcessor::init() { 

   // usually a good idea to
   printParameters() ;

   _nRun = 0 ;
   _nEvt = 0 ;
   
   Global::EVENTSEEDER->registerProcessor(this);
  

}


void FTDBackgroundProcessor::processRunHeader( LCRunHeader* run) { 
   _nRun++ ;
} 

void FTDBackgroundProcessor::processEvent( LCEvent * evt ) { 

   
   /**********************************************************************************************/
   /*       Read in or create the tracker hit collections                                        */
   /**********************************************************************************************/
   
   LCCollection* colStrip = NULL;
   LCCollection* colPixel = NULL ;

   try{
      
      colStrip = evt->getCollection( _colNameFTDStripTrackerHit );
      
   }    
   catch(DataNotAvailableException &e){ // there is no collection with that name so far
      
      
      streamlog_out( DEBUG4 ) << " Collection " << _colNameFTDStripTrackerHit  
                              << " not found -> make a new one ! " << std::endl ;
      
      
      colStrip = new LCCollectionVec( LCIO::TRACKERHITPLANE ); // create a new collection
      evt->addCollection( colStrip , _colNameFTDStripTrackerHit ) ; //and add it to the event
      
   }
   
   try{
      
      colPixel = evt->getCollection( _colNameFTDPixelTrackerHit );
      
   }    
   catch(DataNotAvailableException &e){
      
      
      streamlog_out( DEBUG4 ) << " Collection " << _colNameFTDPixelTrackerHit  
      << " not found -> make a new one ! " << std::endl ;
      
      colPixel = new LCCollectionVec( LCIO::TRACKERHITPLANE );
      evt->addCollection( colPixel , _colNameFTDPixelTrackerHit ) ; 
      
   }
  
  
   /**********************************************************************************************/
   /*       Get gear parameters and seed                                                         */
   /**********************************************************************************************/

   const gear::FTDParameters& ftdParams = Global::GEAR->getFTDParameters() ;
   const gear::FTDLayerLayout& ftdLayers = ftdParams.getFTDLayerLayout() ;
   int nLayers = ftdLayers.getNLayers();


   unsigned seed = Global::EVENTSEEDER->getSeed(this);   
   streamlog_out( DEBUG4 ) << "seed set to " << seed << "\n";
   CLHEP::HepRandom::setTheSeed( seed );

   
   
   CellIDEncoder<TrackerHitPlaneImpl> cellid_encoder( ILDCellID0::encoder_string , colPixel ) ;
  
   
   /**********************************************************************************************/
   /*       Iterate over all sensors and create hits                                             */
   /**********************************************************************************************/

   unsigned backgroundHitsTotal=0;
   
   for ( int side = -1; side <= 1; side+= 2 ){ //for both sides
      
      for ( int layer = 0; layer < nLayers; layer++ ){ //over all layers
         
         
         unsigned nModules = ftdLayers.getNPetals( layer );
         unsigned nSensors = ftdLayers.getNSensors( layer );
         bool isDoubleSided = ftdLayers.isDoubleSided( layer );
         if( isDoubleSided ) assert( nSensors%2 == 0 ); // make sure there is an even number of sensors if doublesided
         
         // area of one petal
         double petalLengthMin = ftdLayers.getSensitiveLengthMin(layer);
         double petalLengthMax = ftdLayers.getSensitiveLengthMax(layer);
         double petalWidth = ftdLayers.getSensitiveWidth(layer);
         double petalRMin = ftdLayers.getSensitiveRinner(layer);
         
         
         unsigned nSensorsOn1Side = nSensors; // how many sensors there are on one side of the petal
         if( isDoubleSided ) nSensorsOn1Side = nSensorsOn1Side / 2;
         
         // The differences of the length for each sensor.
         double deltaLength = (petalLengthMax-petalLengthMin) / double( nSensorsOn1Side ); 
         
         // For example consider a petal with 2 sensors on one side:
         //
         //     --------------  3
         //     \            /
         //     \           /
         //     ------------ 2.5
         //      \        /
         //      \       /
         //      -------- 2
         //
         // Let the longer side have the length 3 (in whatever units) and the shorter one the length 2
         // Then deltaLength would be 0.5: that's how the length changes for every new petal.
         
         double sensorWidth = petalWidth / double( nSensorsOn1Side );
         
         const int sensorType = ftdLayers.getSensorType(layer);
         
         unsigned nHitsOnThisLayer=0;
         
         
         for( unsigned petal = 0; petal < nModules; petal++ ){ //over all petals
            
            
            
            
            double petalPhi = ftdLayers.getPhiPetalCd( layer, petal );
            
            std::vector< double > densities;
            
            
            for( unsigned sensor = 1; sensor <= nSensors; sensor++ ){ // over all sensors
               
               
               cellid_encoder[ ILDCellID0::subdet ] = ILDDetID::FTD  ;
               cellid_encoder[ ILDCellID0::side   ] = side ;
               cellid_encoder[ ILDCellID0::layer  ] = layer ;
               cellid_encoder[ ILDCellID0::module ] = petal ;
               cellid_encoder[ ILDCellID0::sensor ] = sensor ;
               
               gear::MeasurementSurface const* ms = Global::GEAR->getMeasurementSurfaceStore().GetMeasurementSurface( cellid_encoder.lowWord() );
               
               
               unsigned nthSensorOnThisSide = (sensor-1)%nSensorsOn1Side; // the -1 is because sensors start with 1 (and not with 0)
               
               double sensorLengthMin = petalLengthMin + nthSensorOnThisSide * deltaLength;
               double sensorLengthMax = sensorLengthMin + deltaLength;
               double sensorRMin      = petalRMin + nthSensorOnThisSide * deltaLength;  
               double sensorZ         = side * ftdLayers.getSensitiveZposition( layer, petal, sensor );
               
               double area = (sensorLengthMin + sensorLengthMax) * sensorWidth / 2. / 100.; // the area of the sensor in cm^2
               
               
               //generate a random density that's gaussian smeared around the average.
               //And of course multiply the density with the number of integrations. (If we do this later at nHits, we will
               //for lets say 100 integrations always a multiple of 100)
               double density =  CLHEP::RandGauss::shoot( _densityRegulator * _backgroundDensity[layer] * _integratedBX[ layer ] , _densityRegulator * _backgroundDensitySigma[layer] * _integratedBX[ layer ] );
               
               
               /* The following is for doublesided sensors:
                * If we find a number of hits for the front sensor, then the back sensor should have the same
                * number of hits (asuming the hits come from background particles and not detector errors)
                * Therefore we save the densities and for back sensors we simply take the value from the front sensor.
                * This is especially important for strip detectors and low background, because otherwise it would
                * be often the case that the front has e.g. 2 hits and the back 0. 
                * The spacepointbuilder can't make a spacepoint from that, there's nothing to overlap.
                */
               densities.push_back( density );
               if( isDoubleSided && sensor > nSensorsOn1Side )density = densities.at( sensor - nSensorsOn1Side -1 );
               
               
               //calculate the number of hits corresponding to the density on the sensor
               unsigned nHits = unsigned( fabs( area*density ) ) ; // hit = density * area
               
               streamlog_out( DEBUG1 ) << "Placing " << nHits << " hits on: side"  
               << side << " layer"<< layer<< " petal" << petal << " sensor" << sensor << "\n";
               
               
               
               //So now we have the number of hits --> distribute them on the sensor
               for (unsigned int iHit=0; iHit < nHits; iHit++){
                  
                  
                  
                  TrackerHitPlaneImpl* trkHit = new TrackerHitPlaneImpl ;        
                 
                  
                  cellid_encoder.setCellID( trkHit ) ;
                  
                  // get a random position for the hit
                  
                  CLHEP::Hep3Vector globalPos;
                  bool posOkay = false; 
                  
                  for( unsigned k=0; k < 100; k++ ){
                     
                     
                     globalPos = getRandPosition( sensorRMin, sensorLengthMin, sensorLengthMax, sensorWidth, petalPhi, sensorZ );
                     if( k >= 1 ) streamlog_out( DEBUG2 ) << "Retry number " << k << ", side" 
                        << side << " layer"<< layer<< " petal" << petal << " sensor" << sensor << "\n";
                     
                     if ( ms->isGlobalInBoundary( globalPos ) ){
                        
                        posOkay = true;
                        break;
                        
                     }
                     
                  }
                  
                  if( posOkay == false ){
                     
                     streamlog_out( ERROR ) << "After 100 tries the position was still not in the boundary!"
                       << side << " layer"<< layer<< " petal" << petal << " sensor" << sensor << "\n";
                     delete trkHit;
                     continue;
                     
                  }
                  
                  double pos[] = { globalPos.x(), globalPos.y(), globalPos.z() };
                  
                  trkHit->setPosition( pos ) ;
                  
                  
                  gear::CartesianCoordinateSystem* cartesian = dynamic_cast< gear::CartesianCoordinateSystem* >( ms->getCoordinateSystem() ); 
                  CLHEP::Hep3Vector uVec = cartesian->getLocalXAxis();
                  CLHEP::Hep3Vector vVec = cartesian->getLocalYAxis();
                  
                  float u_direction[2] ;
                  u_direction[0] = uVec.theta();
                  u_direction[1] = uVec.phi();
                  
                  float v_direction[2] ;
                  v_direction[0] = vVec.theta();
                  v_direction[1] = vVec.phi();
                  
                  streamlog_out(DEBUG3) 
                  << " U[0] = "<< u_direction[0] << " U[1] = "<< u_direction[1] 
                  << " V[0] = "<< v_direction[0] << " V[1] = "<< v_direction[1]
                  << std::endl ;
                  
                  trkHit->setU( u_direction ) ;
                  trkHit->setV( v_direction ) ;
                  
                  trkHit->setdU( _resU ) ;
                  trkHit->setdV( _resV ) ;
                  
                  
                  
                  if( sensorType == gear::FTDParameters::STRIP ){
                     
                     trkHit->setType( UTIL::set_bit( trkHit->getType() , UTIL::ILDTrkHitTypeBit::ONE_DIMENSIONAL ) ) ;
                     trkHit->setdV( 0 ); // no error in v direction for strip hits as there is no meesurement information in v direction
                     
                     colStrip->addElement( trkHit ); 
                     nHitsOnThisLayer++;
                     
                  }
                  if( sensorType == gear::FTDParameters::PIXEL ){
                     
                     colPixel->addElement( trkHit ); 
                     nHitsOnThisLayer++;
                     
                  }
                  
                  
                  
               }
               
            }
            
         }
         
         backgroundHitsTotal += nHitsOnThisLayer;
         streamlog_out( DEBUG4 ) << "hits on side " << side << " layer " << layer << ": " << nHitsOnThisLayer << "\n";
         
      }
      
   }
   
   streamlog_out( DEBUG4 ) << "Total added background hits on all layers: " << backgroundHitsTotal << "\n";
   
   
  
  
  _nEvt ++ ;
}



  void FTDBackgroundProcessor::check( LCEvent * evt ) { 
  // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void FTDBackgroundProcessor::end(){ 
  
   std::cout << "FTDBackgroundProcessor::end()  " << name() 
            << " processed " << _nEvt << " events in " << _nRun << " runs "
            << std::endl ;
}


CLHEP::Hep3Vector FTDBackgroundProcessor::getRandPosition( double rMin, double lengthMin, double lengthMax, double width, double phi, double z ){
   
   
   CLHEP::Hep3Vector pos;
   
   pos.setZ( z );
   
   
   // now we want an x and a y position.
   // As hits are not evenly distributed in xy, there will be more hits near the beam,
   // this simple approach might be alright:
   // --> make a trapezoid with same shape centered around the x axis and with the bottom sitting at x = 0,
   // --> get a evenly distributed random number for x between the bottom and the top
   // --> calculate the y - width at this x
   // --> get a evenly distributed random number for y between left side and right side
   // --> transfom this coordinates to those of the actual trapezoid
   
   
   double x = CLHEP::RandFlat::shoot ( 0. ,width ); 
   double yWidth = lengthMin + (lengthMax-lengthMin) * ( x / width );
   
   double y = CLHEP::RandFlat::shoot ( -1.*yWidth/2. , yWidth/2. ); 
   
   // now transform to actual trapezoid
   x += rMin;
   
   // now it has the right distance from 0. But it still needs rotation
   double R = sqrt( x*x + y*y );
   double phiStart = atan2( y, x );
   
   phiStart += phi;
   
   
   x = R* cos(phi);
   y = R* sin(phi);
   
   pos.setX(x);
   pos.setY(y);
   
   return pos; 
   
   
   
}



