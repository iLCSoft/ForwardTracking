#include "FTDGhostProcessor.h"

#include <iostream>

#include <EVENT/LCCollection.h>
#include <IMPL/LCCollectionVec.h>

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
                               (int) 1000 ) ;                               
 
   std::vector < int > defaultIsStrip;
   
   defaultIsStrip.push_back ( 0 );
   defaultIsStrip.push_back ( 0 );
   defaultIsStrip.push_back ( 1 );
   defaultIsStrip.push_back ( 1 );
   defaultIsStrip.push_back ( 1 );
   defaultIsStrip.push_back ( 1 );
   defaultIsStrip.push_back ( 1 );
   
   registerProcessorParameter( "IsStrip" ,
                               "Vector or bools (tarned as ints) if a disk is a strip detector (1) or not (0)" ,
                               _isStrip ,
                               defaultIsStrip );                               
                               
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
  


 
   
   std::map < int , std::vector < TrackerHit* > > area; // a map linking the cellID code to a vector of hits, 
                                                                 //that are supposed to be in the area defined by the code
   std::map < int , std::vector < TrackerHit* > >::iterator it;
   
   
   if( col != NULL ){

      int nHits = col->getNumberOfElements()  ;
      
    
      // Sort all the hits according to their cellID0 code into the map "area"
      
      for ( int i=0; i < nHits; i++ ){ //for all hits in the collection
         
       
         TrackerHit* hit = dynamic_cast <TrackerHit*>( col->getElementAt (i) );
      
         const int cellID1 = hit->getCellID1();
         
        
         
         //find out the layer
      
         UTIL::BitField64  cellID( ILDCellID0::encoder_string );
         
         cellID.setValue( cellID1 );

         int layer  = cellID[ ILDCellID0::layer ];
         

         
         if ( _isStrip[ layer ]  ){ //only make ghosts for strip detectors
         

            
            std::vector < TrackerHit* > emptyHitVec;
            
            // create the element in the map (if it already exists nothing happens. So anyway after this we have the value in the map)
            area.insert( pair< int , std::vector < TrackerHit* > >( cellID1 , emptyHitVec ) ); 
            
            area[ cellID1 ].push_back( hit ); //store the hit in the vector
            

         }
         
      }
      
      /**********************************************************************************************/
      /*     Now have a look at all entries in area and make the ghost hits for every CellID        */
      /**********************************************************************************************/
      
      
      
      unsigned int allGhosts=0;         // all ghost hits from all CellIDs
      unsigned int allTrueHits=0;       // all true hits from all CellIDs that are used to create ghost hits (so it's less than the overall true hits!!!)
      
      CellIDEncoder<TrackerHitPlaneImpl> newCellid( ILDCellID0::encoder_string , col ) ;
      
      for ( it = area.begin(); it != area.end(); it++ ){ //over all entries in the map 
         
         unsigned int nTrueHits = 0; //all true hits for this CellID0
         unsigned int nGhosts = 0;
         unsigned int nHits = 0;


         // Those sets correspond to the strips.
         // The integers correspond to the number of the strip that got activated
         // Sets, because a strip can't be activated twice, either it is or it isn't. (that's of course a little simplyfied)
         std::set < int > radial; 
         std::set < int > angular;

         vector < vector <int> > hitStripNumbers; // here the strip numbers of the real hits are stored. 
                                                  // In the inner vector first the radial strip number, then the angular is stored
      
         std::vector < TrackerHit* > hits = it->second; // the values of the map "area" are vectors of the hits that share the CellID stored in the key
         
         

      
         if ( hits.size() > 1 ){ // there are only ghost hits, when more than 1 hit happens in a sensor
            
           
            
            BitField64  cellID( ILDCellID0::encoder_string );
            
            cellID.setValue( it->first );
            
            int side   = cellID[ ILDCellID0::side   ];
            int layer  = cellID[ ILDCellID0::layer  ];
            int module = cellID[ ILDCellID0::module ];
            int sensor = cellID[ ILDCellID0::sensor ];

            // the inner and outer radius of the sensor
            double rSensorMin = _diskInnerRadius[layer] + (_diskOuterRadius[layer] - _diskInnerRadius[layer])/_nSensorsPerPetal*sensor;
            double rSensorMax = rSensorMin + (_diskOuterRadius[layer] - _diskInnerRadius[layer])/_nSensorsPerPetal;
            
            
            // get the numbers of the strips and save them
            for ( unsigned int j=0; j < hits.size(); j++ ){ // over all hits in the area
            
               
               //first: find out the integers corresponding to their place.
               
               const double* pos = hits[j]->getPosition();
               
               streamlog_out( DEBUG2 ) << "\nTrueHit Position: ( " << pos[0] <<" , " << pos[1] << " , " << pos[2] << " )";
               streamlog_out( DEBUG2 ) << " side = " << side 
               << ", layer = " << layer
               << ", module = " << module
               << ", sensor = " << sensor;
               
               double angle = atan2 ( pos[1] , pos[0] ); //the phi angle of the hit in the xy plane
               if (angle < 0.) angle += 2*M_PI; // to the range of 0-2Pi
               
               double angleRel = ( angle - module*2.*M_PI/ _nPetalsPerDisk ) * _nPetalsPerDisk / 2. /M_PI; //a value between 0 and 1
                                          // 0 == at the angle at the begin of the petal
                                          // 1 == at the angle at the end of the petal
               
               int radialStrip = int (  angleRel * _nStripsPerSensor  ); //the number of the radial strip that's activated by this hit
               if (radialStrip == _nStripsPerSensor) radialStrip--;
               
               
               double r = sqrt ( pos[0]*pos[0] + pos[1]*pos[1] );

               double rRel = ( r - rSensorMin ) / (rSensorMax - rSensorMin); //the relative radius of the hit in the sensor.
                                 // between 0 and 1
                                 // 0 == at the inner radius of the sensor
                                 // 1 == at the outer radius of the sensor
               
               int angularStrip = int (rRel * _nStripsPerSensor); //number of the activated angular strip
               if (angularStrip == _nStripsPerSensor) angularStrip--;
               
               
               
               // So now we know what numbers the strips have, so let's save that
               
               radial.insert( radialStrip );    //Store the activated strips
               angular.insert( angularStrip );
               
               std::vector < int > newNumbers; 
               newNumbers.push_back( radialStrip );
               newNumbers.push_back( angularStrip );
               
               hitStripNumbers.push_back( newNumbers ); //store the hits that caused the activation (so we don't add them later as additional ghost hits)
               
            }
            
            
            // Now calculate all the ghosthits
            std::set < int >::iterator itRad;
            std::set < int >::iterator itAng;
            
            for ( itRad = radial.begin(); itRad != radial.end(); itRad++){ //Over all radial strips
               
               for ( itAng = angular.begin(); itAng!= angular.end(); itAng++){ //Over all angular strips
                  
                  bool isGhostHit = true;
                  
                  //check if this corresponds to a real hit (because: as said before: we don't want to add a real hit again)
                  for ( unsigned int k=0; k< hitStripNumbers.size(); k++){ //check all real hits
                     
                     if (( hitStripNumbers[k][0]== *itRad) && ( hitStripNumbers[k][1]== *itAng)) //it is a real hit
                        isGhostHit = false;
                     
                  }
                  
                  if ( isGhostHit ){ //Only for ghost hits, we don't want to save the real hits a second time
                     
                     // calculate the position of the hit
                     double ang = (double) *itAng; // the angular number
                     double rad = (double) *itRad; // the radial number
                     
                     // the radius of the hit in the xy plane
                     double r = rSensorMin + (rSensorMax - rSensorMin)* ang / (double) _nStripsPerSensor; 
                     
                     // the phi angle in the xy plane
                     double phi = module * 2. * M_PI / (double) _nPetalsPerDisk + rad / 
                                  (double) _nStripsPerSensor * 2 * M_PI/ (double)_nPetalsPerDisk;
                                  
              
                     
                     double pos[3] = {0. , 0. , 0.};
                     pos[0] = r * cos(phi);     //x
                     pos[1] = r * sin(phi);     //y
                     pos[2] = _diskPositionZ[ layer ]; // the z postion is exact
                     
                     // smear the hit position
                     pos[0] = CLHEP::RandGauss::shoot( pos[0] , _pointReso );
                     pos[1] = CLHEP::RandGauss::shoot( pos[1] , _pointReso );
                     
                     streamlog_out( DEBUG2 ) << "\nGhostHit Position: ( " << pos[0] <<" , " << pos[1] << " , " << pos[2] << " )";
                     streamlog_out( DEBUG2 ) << " side = " << side 
                                             << ", layer = " << layer
                                             << ", module = " << module
                                             << ", sensor = " << sensor;
                     
                     
                     //So now make the Hit
                     TrackerHitPlaneImpl* ghostHit = new TrackerHitPlaneImpl ;
                     
                     ghostHit->setType( 201+layer);  // needed for FullLDCTracking et al.


                     
                     newCellid[ ILDCellID0::subdet ] = ILDDetID::FTD ;
                     newCellid[ ILDCellID0::side   ] = side ;
                     newCellid[ ILDCellID0::layer  ] = layer ;
                     newCellid[ ILDCellID0::module ] = module ;
                     newCellid[ ILDCellID0::sensor ] = sensor ;
                     
                     
                     // This is needed, cause MarlinTrk needs a correct CellID0. Therefore we store our stuff in
                     // CellID1
                     newCellid.setValue( lcio::long64(newCellid.lowWord() ) << 32 );
                     
                     newCellid[ ILDCellID0::subdet ] = ILDDetID::FTD ;
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
            
            
            
            streamlog_out( DEBUG3 ) << "\n Added ghost hits on layer " << (layer+1)*side 
                                    << ", Petal " << module
                                    << ", Sensor " << sensor
                                    << "\n\t trueHits: " << nTrueHits
                                    << "\n\t GhostHits: " << nGhosts
                                    << "\n\t AllHits: " << nHits
                                    << "\n";
                                    
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


