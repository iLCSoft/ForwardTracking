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
#include <set>

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
                               

   registerProcessorParameter( "nStripsPerSensor" ,
                               "Number of Strips per sensor" ,
                               _nStripsPerSensor ,
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
 
   CellIDEncoder<TrackerHit> cellid_encoder(  ILDCellID0::encoder_string , col );
  
   
   std::map < lcio::long64 , std::vector < TrackerHit* > > area;
   std::map < lcio::long64 , std::vector < TrackerHit* > >::iterator it;
   
   
   if( col != NULL ){

      int nHits = col->getNumberOfElements()  ;
      
    
      // Sort all the hits according to theyr cellID0 code into the map "area"
      for ( int i=0; i < nHits; i++ ){ //for all hits in the collection
         
       
         TrackerHit* hit = dynamic_cast <TrackerHit*>( col->getElementAt (i) );
      
         const double* pos = hit->getPosition();
         
         //find out direction
         int side = round( pos[2] / fabs( pos[2] ) );
         
         //find out the layer
         
         int layer = 0;
         
         for ( unsigned int j=1; j < _diskPositionZ.size(); j++ ){
            
            if ( fabs( pos[2] ) > ( _diskPositionZ[j] + _diskPositionZ[j-1] )/2. ){
               
               layer = j;
            }
           
         }
         
         if ( layer >= 2 ){ //inner 2 disks are pixels --> no ghosts there TODO: make this a steering parameter
         
            //find out the petal (= modul)
            
            double angle = atan2 ( pos[1] , pos[0] ); 
            if (angle < 0.) angle += 2*M_PI; // to the range of 0-2Pi
            
            double angleRel = angle / 2. /M_PI;
            
            int petal = floor ( angleRel * _nPetalsPerDisk );
            if ( petal >= _nPetalsPerDisk ) petal = _nPetalsPerDisk - 1; //just in case, the hit really is at the exact boarder of a petal
            
            //find out the sensor
            
            double r = sqrt ( pos[0]*pos[0] + pos[1]*pos[1] );
      
            double posRel = (r - _diskInnerRadius[layer]) / ( _diskOuterRadius[layer] - _diskInnerRadius[layer] ); 
            
            int sensor = floor ( posRel * _nSensorsPerPetal );
            if ( sensor >= _nSensorsPerPetal ) sensor = _nSensorsPerPetal -1; //just in case, the hit really is at the exact boarder
            

            // Now we have the information where the hit belongs.
            //So set up the code
            cellid_encoder[ ILDCellID0::subdet ] = ILDDetID::FTD  ;
            cellid_encoder[ ILDCellID0::side   ] = side ;
            cellid_encoder[ ILDCellID0::layer  ] = layer ;
            cellid_encoder[ ILDCellID0::module ] = petal ;
            cellid_encoder[ ILDCellID0::sensor ] = sensor ;
            
            lcio::long64 cellID = cellid_encoder.getValue();
            
            std::vector < TrackerHit* > emptyHitVec;
            //create the element in the map (if it already exists nothing happens. So anyway after this we habe the value in the map
            area.insert( pair< lcio::long64 , std::vector < TrackerHit* > >( cellID , emptyHitVec ) ); 
            
            area[ cellID ].push_back( hit );
            
//             streamlog_out( DEBUG4 ) << "\n Saved real hit from layer " << (layer+1)*side 
//                            << ", Petal " << petal
//                            << ", Sensor " << sensor
//                            << ", Position: ( " << pos[0] <<" , " << pos[1] << " , " << pos[2] << " )"
//                            << "\n";
         }
         
      }
      
      
      
      //Now enter all entries in area and make the ghost hits
      
      unsigned int allGhosts=0;
      unsigned int allTrueHits=0;
      
      CellIDEncoder<TrackerHitPlaneImpl> newCellid( ILDCellID0::encoder_string , col ) ;
      
      for ( it = area.begin(); it != area.end(); it++ ){ //over all entries in the map 
         
         unsigned int nTrueHits = 0;
         unsigned int nGhosts = 0;
         unsigned int nHits = 0;


         //Those sets correspond to the strips
         std::set < int > radial;
         std::set < int > angular;

         vector < vector <int> > hitStripNumbers; //here the Strip numbers of the real hits are stored. 
                                                   // In the inner vector first the radial, then the angular is stored
      
         std::vector < TrackerHit* > hits = it->second;
         
         

      
         if ( hits.size() > 1 ){
            
            BitField64 b("subdet:5,side:-2,layer:9,module:8,sensor:8,fill:32" );
            b.setValue ( it->first ); //does that work that way?
            int petal = b[ "module" ];
            int sensor = b[ "sensor" ];
            int layer = b[ "layer" ];
            int side = b[ "side" ];
            

            
            double rSensorMin = _diskInnerRadius[layer] + (_diskOuterRadius[layer] - _diskInnerRadius[layer])/_nSensorsPerPetal*sensor;
            double rSensorMax = rSensorMin + (_diskOuterRadius[layer] - _diskInnerRadius[layer])/_nSensorsPerPetal;
            
            
            // get the numbers of the strips and save them
            for ( unsigned int j=0; j < hits.size(); j++ ){ // over all hits in the area
            
               
               //first: find out the integers corresponding to their place.
               
               const double* pos = hits[j]->getPosition();
               
//                streamlog_out( DEBUG4 ) << "\nTrueHit Position: ( " << pos[0] <<" , " << pos[1] << " , " << pos[2] << " )";
               
               double angle = atan2 ( pos[1] , pos[0] ); 
               if (angle < 0.) angle += 2*M_PI; // to the range of 0-2Pi
               
               double angleRel = ( angle - petal*2.*M_PI/ _nPetalsPerDisk ) * _nPetalsPerDisk / 2. /M_PI; //should now be a value between 0 and 1
               
               int radialStrip = floor ( angleRel * _nStripsPerSensor );
               if (radialStrip == _nStripsPerSensor) radialStrip--;
               
               
               double r = sqrt ( pos[0]*pos[0] + pos[1]*pos[1] );

               double rRel = ( r - rSensorMin ) / (rSensorMax - rSensorMin);
               
               int angularStrip = floor (rRel * _nStripsPerSensor);
               if (angularStrip == _nStripsPerSensor) angularStrip--;
               
               
               
               // So now we know what numbers the strips have, so let's save that
               
               radial.insert( radialStrip );
               angular.insert( angularStrip );
               
               std::vector < int > newNumbers; 
               newNumbers.push_back( radialStrip );
               newNumbers.push_back( angularStrip );
               
               hitStripNumbers.push_back( newNumbers );
               
            }
            
            
            // calculate all the ghosthits
            std::set < int >::iterator itRad;
            std::set < int >::iterator itAng;
            
            for ( itRad = radial.begin(); itRad != radial.end(); itRad++){
               
               for ( itAng = angular.begin(); itAng!= angular.end(); itAng++){
                  
                  bool isGhostHit = true;
                  
                  //check if this corresponds to a real hit
                  for ( unsigned int k=0; k< hitStripNumbers.size(); k++){
                     
                     if (( hitStripNumbers[k][0]== *itRad) && ( hitStripNumbers[k][1]== *itAng))
                        isGhostHit = false;
                     
                  }
                  
                  if ( isGhostHit ){ //Only for ghost hits, we don't want to save the real hits a second time
                     
                     //calculate the position of the hit
                     double ang = (double) *itAng;
                     double rad = (double) *itRad;
                     
                     double r = rSensorMin + (rSensorMax - rSensorMin)* ang / (double) _nStripsPerSensor;
                     
                     double phi = petal * 2. * M_PI / (double) _nPetalsPerDisk + rad / 
                                  (double) _nStripsPerSensor * 2 * M_PI/ (double)_nPetalsPerDisk;
                                  
              
                     
                     double pos[3] = {0. , 0. , 0.};
                     pos[0] = r * cos(phi);
                     pos[1] = r * sin(phi);
                     pos[2] = _diskPositionZ[ layer ];
                     
//                      streamlog_out( DEBUG4 ) << "\nGhostHit Position: ( " << pos[0] <<" , " << pos[1] << " , " << pos[2] << " )";
                     
                     
                     //TODO: SMEAR THE HITS
                     
                     
                     //So now make the Hit
                     TrackerHitPlaneImpl* ghostHit = new TrackerHitPlaneImpl ;
                     
                     ghostHit->setType( 201+layer);  // needed for FullLDCTracking et al.

                     newCellid[ ILDCellID0::subdet ] = ILDDetID::FTD  ;
                     newCellid[ ILDCellID0::side   ] = side ;
                     newCellid[ ILDCellID0::layer  ] = layer ;
                     newCellid[ ILDCellID0::module ] = 0 ;
                     newCellid[ ILDCellID0::sensor ] = 0 ;
                     
                     newCellid.setCellID( ghostHit ) ;

                     ghostHit->setPosition(  pos  ) ;

                     float u_direction[2] ; // x
                     u_direction[0] = 0.0 ; 
                     u_direction[1] = M_PI/2.0 ;
                     
                     float v_direction[2] ; // y
                     v_direction[0] = M_PI/2.0 ;
                     v_direction[1] = M_PI/2.0 ;
                     
                     ghostHit->setU( u_direction ) ;
                     ghostHit->setV( v_direction ) ;
                     
                     ghostHit->setdU( _pointReso ) ;
                     ghostHit->setdV( _pointReso ) ;
                              
                     ghostHit->setEDep( 0. ) ;

                     col->addElement( ghostHit ) ; 
                     
                     nGhosts++;
                     nHits++;
                     


                  }
                  else{ // it was a real hit
                   
                     nTrueHits++;
                     nHits++;
                     
                  }
                  
               }
               
            }
            
                        // Output the results
            
//             streamlog_out( DEBUG4 ) << "\n Added ghost hits on layer " << (layer+1)*side 
//                                     << ", Petal " << petal
//                                     << ", Sensor " << sensor
//                                     << "\n\t trueHits: " << nTrueHits
//                                     << "\n\t GhostHits: " << nGhosts
//                                     << "\n\t AllHits: " << nHits
//                                     << "\n";
                                    
            allGhosts += nGhosts;
            allTrueHits += nTrueHits;
                                    
                                    
         }
         
         

         
      }
      
      
      streamlog_out( DEBUG4 ) << "\n\nCreated " << allGhosts << " ghost hits from " << allTrueHits << " original hits.\n";
      

   }
  
  
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