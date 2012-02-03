#include "ForwardTracking.h"

#include <algorithm>

#include <EVENT/TrackerHit.h>
#include <EVENT/Track.h>
#include <EVENT/LCCollection.h>
#include <IMPL/LCCollectionVec.h>
#include "IMPL/LCFlagImpl.h"


#include "marlin/VerbosityLevels.h"

#include <MarlinCED.h>

#include <gear/GEAR.h>
#include <gear/GearParameters.h>
#include <gear/BField.h>
#include "gear/FTDParameters.h"
#include "gear/FTDLayerLayout.h"


//--------------------------------------------------------------
#include "FTDTrack.h"
#include "FTrackTools.h"
#include "TrackSubsetHopfieldNN.h"
#include "TrackSubsetSimple.h"
#include "SegmentBuilder.h"
#include "Automaton.h"
#include "FTDHit01.h"
#include "FTDNeighborPetalSecCon.h"
#include "FTDSecCon01.h"
//--------------------------------------------------------------


using namespace lcio ;
using namespace marlin ;
using namespace FTrack;
using namespace MarlinTrk ;





ForwardTracking aForwardTracking ;


ForwardTracking::ForwardTracking() : Processor("ForwardTracking") {

   // modify processor description
   _description = "ForwardTracking reconstructs tracks through the FTDs" ;


   // register steering parameters: name, description, class-variable, default value
   registerInputCollection(LCIO::TRACKERHIT,
                           "FTDHitCollectionName",
                           "FTD Hit Collection Name",
                           _FTDHitCollection,
                           std::string("FTDTrackerHits")); 


   registerOutputCollection(LCIO::TRACK,
                           "ForwardTrackCollection",
                           "Name of the Forward Tracking output collection",
                           _ForwardTrackCollection,
                           std::string("ForwardTracks"));

   
   registerProcessorParameter("Chi2ProbCut",
                              "The chi2 probability value below which tracks will be cut",
                              _chi2ProbCut,
                              double(0.005));
   
   //For fitting:
   
   registerProcessorParameter("MultipleScatteringOn",
                              "Use MultipleScattering in Fit",
                              _MSOn,
                              bool(true));
   
   registerProcessorParameter("EnergyLossOn",
                              "Use Energy Loss in Fit",
                              _ElossOn,
                              bool(true));
   
   registerProcessorParameter("SmoothOn",
                              "Smooth All Measurement Sites in Fit",
                              _SmoothOn,
                              bool(false));
   
   /////////////////////////////////////////////////////////////////////////////////
   // The Criteria for the Cellular Automaton
   
   std::vector< std::string > allCriteria = Criteria::getAllCriteriaNamesVec();
   
   
   registerProcessorParameter( "Criteria",
                               "A vector of the criteria that are going to be used. For every criterion a min and max needs to be set!!!",
                               _criteriaNames,
                               allCriteria);
   
   for( unsigned i=0; i < _criteriaNames.size(); i++ ){
    
      
      std::string critMinString = _criteriaNames[i] + "_min";
      
      registerProcessorParameter( critMinString,
                                  "The minimum of " + _criteriaNames[i],
                                  _critMinima[ _criteriaNames[i] ],
                                  float( 0. ));
      
      
      std::string critMaxString = _criteriaNames[i] + "_max";
      
      registerProcessorParameter( critMaxString,
                                  "The maximum of " + _criteriaNames[i],
                                  _critMaxima[ _criteriaNames[i] ],
                                  float( 0. ));
      
   
   }
   
   /////////////////////////////////////////////////////////////////////////////////
   
}




void ForwardTracking::init() { 

   streamlog_out(DEBUG) << "   init called  " << std::endl ;

   // usually a good idea to
   printParameters() ;

   _nRun = 0 ;
   _nEvt = 0 ;

   _useCED = false;
   if( _useCED )MarlinCED::init(this) ;    //CED
   

   // Get data from gear
   unsigned int nLayers = 8; // layer 0 is for the IP
   unsigned int nModules = 1;
   unsigned int nSensors = 2; // there is at the moment only one sensor, namely sensor 1, but as usually things start with 0...
  
   const gear::FTDParameters& ftdParams = Global::GEAR->getFTDParameters() ;
   const gear::FTDLayerLayout& ftdLayers = ftdParams.getFTDLayerLayout() ;
   nLayers = ftdLayers.getNLayers() + 1;
   nModules = ftdLayers.getNPetals(0); 
   
   streamlog_out( DEBUG4 ) << "using " << nLayers - 1 << " layers, " << nModules << " petals and " << nSensors << " sensors.\n";

   _Bz = Global::GEAR->getBField().at( gear::Vector3D(0., 0., 0.) ).z();    //The B field in z direction
  
   _sectorSystemFTD = new SectorSystemFTD( nLayers, nModules , nSensors );


   //Initialise the TrackFitter of the tracks:
   FTDTrack::initialiseFitter( "KalTest" , marlin::Global::GEAR , "" , _MSOn , _ElossOn , _SmoothOn  );



   // store the criteria 
   for( unsigned i=0; i<_criteriaNames.size(); i++ ){
      
      std::string critName = _criteriaNames[i];
      
      ICriterion* crit = Criteria::createCriterion( critName, _critMinima[critName] , _critMaxima[critName] );
      
      std::string type = crit->getType();
      
      streamlog_out( DEBUG4 ) <<  "\nAdded: Criterion " << critName << " (type =  " << type 
                              << " ). Min = " << _critMinima[critName]
                              << ", Max = " << _critMaxima[critName];
      
      if( type == "2Hit" ){
         
         _crit2Vec.push_back( crit );
         
      }
      else 
      if( type == "3Hit" ){
         
         _crit3Vec.push_back( crit );
         
      }
      else 
      if( type == "4Hit" ){
         
         _crit4Vec.push_back( crit );
         
      }
      else delete crit;


   }



}


void ForwardTracking::processRunHeader( LCRunHeader* run) { 

    _nRun++ ;
} 



void ForwardTracking::processEvent( LCEvent * evt ) { 


//--CED---------------------------------------------------------------------
// Reset drawing buffer and START drawing collection

   if( _useCED ){
      
      MarlinCED::newEvent(this , 0) ; 
      
      CEDPickingHandler &pHandler=CEDPickingHandler::getInstance();
      
      pHandler.update(evt); 
      
   }

//-----------------------------------------------------------------------
  
//    const gear::GearParameters& paramFTD = Global::GEAR->getGearParameters("FTD");
//    drawFTDSensors( paramFTD , 16 , 2 ); //TODO use the same values here as for the code

   
  

   LCCollection* col = evt->getCollection( _FTDHitCollection ) ;

  
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //                                                                                                              //
   //                                                                                                              //
   //                            Track Reconstruction in the FTDs                                                  //
   //                                                                                                              //
   //                                                                                                              //
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  


   //First: collect all the hits and store them
   if( col != NULL ){

     
      unsigned nHits = col->getNumberOfElements()  ;
      
      
      streamlog_out( DEBUG4 ) << "Number of hits on the FTDs: " << nHits <<"\n";
      
      
      // A map to store the hits according to their sectors
      std::map< int , std::vector< IHit* > > map_sector_hits;
      
      
      std::vector< IHit* > hitsTBD; //Hits to be deleted at the end
      
      for(unsigned i=0; i< nHits ; i++){
         
         
         TrackerHit* trkHit = dynamic_cast<TrackerHit*>( col->getElementAt( i ) );
         
         //Make an FTDHit01 from the TrackerHit 
         FTDHit01* ftdHit = new FTDHit01 ( trkHit , _sectorSystemFTD );
         hitsTBD.push_back(ftdHit);
       
         map_sector_hits[ ftdHit->getSector() ].push_back( ftdHit );         
         
        
      }
      
      
      IHit* virtualIPHitForward = createVirtualIPHit(1 , _sectorSystemFTD );
      hitsTBD.push_back( virtualIPHitForward );
      map_sector_hits[ virtualIPHitForward->getSector() ].push_back( virtualIPHitForward );
      
      IHit* virtualIPHitBackward = createVirtualIPHit(-1 , _sectorSystemFTD );
      hitsTBD.push_back( virtualIPHitBackward );
      map_sector_hits[ virtualIPHitBackward->getSector() ].push_back( virtualIPHitBackward );
      
      ////////////////////////////////////////////////////
      std::map< int , std::vector< IHit* > >::iterator it;
      
      for( it = map_sector_hits.begin(); it != map_sector_hits.end(); it++ ){
         
         
         std::vector<IHit*> hits = it->second;
         int sector = it->first;
         
         int side = _sectorSystemFTD->getSide( sector );
         unsigned layer = _sectorSystemFTD->getLayer( sector );
         unsigned module = _sectorSystemFTD->getModule( sector );
         unsigned sensor = _sectorSystemFTD->getSensor( sector );
         
         streamlog_out( DEBUG2 ) << "\nSECTOR " << sector  << " ("
                                 << side << ","
                                 << layer << ","
                                 << module << ","
                                 << sensor << ") "
                                 << " has " << hits.size() << " hits.";
         
//          for( unsigned i=0; i<hits.size(); i++){
//             
//             streamlog_out( DEBUG4 ) << "\n\t" << hits[i];
//             
//          }
      
      }
      
      /////////////////////////////////////////////////
      
      /**********************************************************************************************/
      /*                Check the possible connections of hits on overlapping petals                */
      /**********************************************************************************************/
      
      std::map< IHit* , std::vector< IHit* > > map_hitFront_hitsBack = getOverlapConnectionMap( map_sector_hits, _sectorSystemFTD, 3.5);
      
      /**********************************************************************************************/
      /*                Build the segments                                                          */
      /**********************************************************************************************/
      
      
      streamlog_out( DEBUG4 ) << "\t\t---SegementBuilder---\n" ;
      
      //Create a segmentbuilder
      SegmentBuilder segBuilder( map_sector_hits );
      
      
      
      segBuilder.addCriteria ( _crit2Vec );
      
      //Also load hit connectors
      unsigned layerStepMax = 2; // how many layers to go at max
      unsigned petalStepMax = 1; // how many petals to go at max
      unsigned lastLayerToIP = 4;// layer 1,2...4 get connected directly to the IP
      FTDSecCon01 secCon( _sectorSystemFTD , layerStepMax , petalStepMax , lastLayerToIP );
      
      
      segBuilder.addSectorConnector ( & secCon );
      
      
      // And get out the 1-segments 
      Automaton automaton = segBuilder.get1SegAutomaton();
      
      
      /**********************************************************************************************/
      /*                Automaton                                                                   */
      /**********************************************************************************************/
      
      
      
      streamlog_out( DEBUG4 ) << "\t\t---Automaton---\n" ;
      
      if( _useCED ) automaton.drawSegments();
      
      /*******************************/
      /*      2-hit segments         */
      /*******************************/
      streamlog_out( DEBUG4 ) << "\t\t--2-hit-Segments--\n" ;
      
      automaton.clearCriteria();
      automaton.addCriteria( _crit3Vec );  
      
      
      // Let the automaton lengthen its 1-segments to 2-segments
      automaton.lengthenSegments();
      
      // So now we have 2-segments and are ready to perform the cellular automaton.
      
      // Perform the automaton
      automaton.doAutomaton();
      
      //Clean segments with bad states
      automaton.cleanBadStates();
      
      //Clean connections of segments (this uses the same criteria again as before)
      automaton.cleanBadConnections();
      
      //Reset the states of all segments
      automaton.resetStates();
      
      
      /*******************************/
      /*      3-hit segments         */
      /*******************************/
      streamlog_out( DEBUG4 ) << "\t\t--3-hit-Segments--\n" ;
      
      
      automaton.clearCriteria();
      automaton.addCriteria( _crit4Vec );      
      
      automaton.lengthenSegments();
      // So now we have 3 hit segments 
      
      // Perform the automaton
      automaton.doAutomaton();

      //Clean segments with bad states
      automaton.cleanBadStates();
       
      //Clean connections of segments (this uses the same criteria again as before)
      automaton.cleanBadConnections();
      
      //Reset the states of all segments
      automaton.resetStates();
      
      
      
      
      
      /**********************************************************************************************/
      /*                Add the overlapping hits                                                    */
      /**********************************************************************************************/
      
      
      streamlog_out( DEBUG4 ) << "\t\t---Add hits from overlapping petals + fit + chi2prob Cuts---\n" ;
      
      std::vector < rawTrack > autRawTracks = automaton.getTracks(3);
      std::vector <ITrack*> trackCandidates;
      
      
      // for all raw tracks
      for( unsigned i=0; i < autRawTracks.size(); i++){
         
         std::vector <ITrack*> overlappingTrackCands;
         std::vector< IHit* > trackHits = autRawTracks[i];
         
         std::vector < rawTrack > autRawTracksPlus;          // the tracks plus the overlapping hits
         autRawTracksPlus.push_back( trackHits );            // add the basic track
         
         
         // for all hits in the track
         for( unsigned j=0; j<trackHits.size(); j++ ){
            
            IHit* hit = trackHits[j];
            
            //get all the possible overlapping hits
            std::map< IHit* , std::vector< IHit* > >::iterator it;
            
            
            it = map_hitFront_hitsBack.find( hit );
            
            if( it == map_hitFront_hitsBack.end() ) continue; // if there are no hits to be added
            
            std::vector< IHit* > overlappingHits = it->second; 
            
            //for all hits to be added
            for( unsigned k=0; k<overlappingHits.size(); k++ ){
               
               
               IHit* oHit = overlappingHits[k];
               
               //for all tracks we have so far create an additional track with the hit
               unsigned nAutTrackHitsPlus = autRawTracksPlus.size();
               for( unsigned l=0; l<nAutTrackHitsPlus; l++ ){
                  
                  std::vector< IHit* > newTrackHits = autRawTracksPlus[l];
                  newTrackHits.push_back( oHit );
                  autRawTracksPlus.push_back( newTrackHits );
                  
               }
               
            }
            
         }
         
         /**********************************************************************************************/
         /*                Make track candidates, fit them and throw away bad ones                     */
         /**********************************************************************************************/
         
         for( unsigned j=0; j < autRawTracksPlus.size(); j++ ){
            
            
            ITrack* trackCand = new FTDTrack( autRawTracksPlus[j] );
            trackCand->fit();
            
            streamlog_out( DEBUG2 ) << " Track " << trackCand 
                                    << " chi2Prob = " << trackCand->getChi2Prob() 
                                    << "( chi2=" << trackCand->getChi2() 
                                    <<", Ndf=" << trackCand->getNdf() << " )\n";
            
            
            if ( trackCand->getChi2Prob() > _chi2ProbCut ){
               
               overlappingTrackCands.push_back( trackCand );
               
            }
            else{
               
               delete trackCand;
               
            }
            
         }
         
         /**********************************************************************************************/
         /*                Take the best version of the track                                          */
         /**********************************************************************************************/
        
         if( !overlappingTrackCands.empty() ){
            
            ITrack* bestTrack = overlappingTrackCands[0];
            
            for( unsigned j=1; j < overlappingTrackCands.size(); j++ ){
               
               if( overlappingTrackCands[j]->getQI() > bestTrack->getQI() ){
                  
                  delete bestTrack; //delete the old one, not needed anymore
                  bestTrack = overlappingTrackCands[j];
               }
               else{
                  
                  delete overlappingTrackCands[j]; //delete this one
                  
               }
               
            }
            
            trackCandidates.push_back( bestTrack );
            
         }
         
      }
      
      
      /**********************************************************************************************/
      /*               Get the best subset of tracks                                                */
      /**********************************************************************************************/
      streamlog_out( DEBUG4 ) << "\t\t---Get best subset of tracks---\n" ;
      
      
      // Make a TrackSubset
//       TrackSubsetSimple subset;
      TrackSubsetHopfieldNN subset;
      subset.addTracks( trackCandidates ); 
      
      //Calculate the best subset:
      subset.calculateBestSet();
      
      std::vector< ITrack* > tracks = subset.getAcceptedTracks();
      std::vector< ITrack* > rejectedTracks = subset.getRejectedTracks();

      // immediately delete the rejected ones
      for ( unsigned i=0; i<rejectedTracks.size(); i++){
         
         delete rejectedTracks[i];
         
      }
      
      
      
      /**********************************************************************************************/
      /*               Finally: save the tracks                                                     */
      /**********************************************************************************************/
      streamlog_out( DEBUG4 ) << "\t\t---Save Tracks---\n" ;
      
      LCCollectionVec * trkCol = new LCCollectionVec(LCIO::TRACK);
      
      LCFlagImpl hitFlag(0) ;
      hitFlag.setBit( LCIO::TRBIT_HITS ) ;
      trkCol->setFlag( hitFlag.getFlag()  ) ;

      for (unsigned int i=0; i < tracks.size(); i++){
         
         FTDTrack* myTrack = dynamic_cast< FTDTrack* >( tracks[i] );
         
         if( myTrack != NULL ) trkCol->addElement( myTrack->getLcioTrack() );
         
         
      }
      evt->addCollection(trkCol,_ForwardTrackCollection.c_str());
      
      
      
      streamlog_out (MESSAGE0) << "\nForward Tracking found and saved " << tracks.size() << " tracks.\n"; 
      
      
      /**********************************************************************************************/
      /*                Clean up                                                                    */
      /**********************************************************************************************/
      
      // delete all the created IHits
      for ( unsigned i=0; i<hitsTBD.size(); i++ )  delete hitsTBD[i];
      
      // delete the FTracks
      for (unsigned int i=0; i < tracks.size(); i++){ delete tracks[i];}
      
      
      
      
   }




   if( _useCED ) MarlinCED::draw(this);


   _nEvt ++ ;
   
}





void ForwardTracking::check( LCEvent * evt ) {}


void ForwardTracking::end(){
   
   for ( unsigned i=0; i< _crit2Vec.size(); i++) delete _crit2Vec[i];
   for ( unsigned i=0; i< _crit3Vec.size(); i++) delete _crit3Vec[i];
   for ( unsigned i=0; i< _crit4Vec.size(); i++) delete _crit4Vec[i];
   _crit2Vec.clear();
   _crit3Vec.clear();
   _crit4Vec.clear();
   
   delete _sectorSystemFTD;
   _sectorSystemFTD = NULL;
   
}

void ForwardTracking::drawFTDSensors ( const gear::GearParameters& paramFTD , unsigned nPetalsPerDisk , unsigned nSensorsPerPetal){
   

   
   std::vector <double> diskPositionZ = paramFTD.getDoubleVals( "FTDZCoordinate" ) ;
   std::vector <double> diskInnerRadius = paramFTD.getDoubleVals( "FTDInnerRadius" ) ;
   std::vector <double> diskOuterRadius = paramFTD.getDoubleVals( "FTDOuterRadius" ) ; 
   
   unsigned int color = 0x9999ff;
   
   
   for ( int side = -1; side <= 1; side +=2){ //for backward and forward

      for ( unsigned int disk=0; disk < diskPositionZ.size(); disk++ ){ //over all disks


         double rMin = diskInnerRadius[ disk ];
         double rMax = diskOuterRadius[ disk ];
         double z      = side * diskPositionZ[ disk ];            
         
         //draw the radial boarders (i.e. straight lines)
         for (unsigned int petal=0; petal < nPetalsPerDisk; petal++){ //over all petals

            double phi = 2 * M_PI / (float) nPetalsPerDisk * (float) petal; //the phi angle of the first boarder of the petal
            
            
            double xStart = rMin * cos( phi );
            double yStart = rMin * sin( phi );
            double xEnd   = rMax * cos( phi );
            double yEnd   = rMax * sin( phi );
            
            
            ced_line_ID( xStart, yStart, z , xEnd , yEnd , z , 2 , 2, color, 2);
            
            
         }
         
         //draw the angular boarders (i.e. circles)
         for (unsigned int i=0; i <= nSensorsPerPetal; i++){
            
            unsigned nLines = 360;
            
            double r = rMin + ( rMax - rMin ) / (float) nSensorsPerPetal * (float) i; //the radius of the circle
            
            for (unsigned int j=0; j<nLines; j++){
               
               double phiStart = 2.*M_PI / nLines * j;
               double phiEnd   = 2.*M_PI / nLines * (j +1);
               
               double xStart = r * cos(phiStart);
               double yStart = r * sin(phiStart);
               double xEnd   = r * cos(phiEnd);
               double yEnd   = r * sin(phiEnd);
               
               ced_line_ID( xStart, yStart, z , xEnd , yEnd , z , 2 , 2, color, 2);
               
            }
            
         }
         
      }
      
   }
   
   
   
   
   
}



std::map< IHit* , std::vector< IHit* > > ForwardTracking::getOverlapConnectionMap( 
            std::map< int , std::vector< IHit* > > & map_sector_hits, 
            const SectorSystemFTD* secSysFTD,
            float distMax){
   
   
   unsigned nConnections=0;

   
   std::map< IHit* , std::vector< IHit* > > map_hitFront_hitsBack;
   std::map< int , std::vector< IHit* > >::iterator it;
   
   //for every sector
   for ( it= map_sector_hits.begin() ; it != map_sector_hits.end(); it++ ){
      
     
      std::vector< IHit* > hitVecA = it->second;
      int sector = it->first;
      // get the neighbouring petals
      FTDNeighborPetalSecCon secCon( secSysFTD );
      std::set< int > targetSectors = secCon.getTargetSectors( sector );
      
      
      //for all neighbouring petals
      for ( std::set<int>::iterator itTarg = targetSectors.begin(); itTarg!=targetSectors.end(); itTarg++ ){
         
         
         std::vector< IHit* > hitVecB = map_sector_hits[ *itTarg ];
         
         for ( unsigned j=0; j < hitVecA.size(); j++ ){
            
            
            IHit* hitA = hitVecA[j];
            
            for ( unsigned k=0; k < hitVecB.size(); k++ ){
               
               
               IHit* hitB = hitVecB[k];
               
               
               float dx = hitA->getX() - hitB->getX();
               float dy = hitA->getY() - hitB->getY();
               float dz = hitA->getZ() - hitB->getZ();
               float dist = sqrt( dx*dx + dy*dy + dz*dz );
               
               if (( dist < distMax )&& ( fabs( hitB->getZ() ) > fabs( hitA->getZ() ) )  ){
                  
                  
                  streamlog_out( DEBUG2 ) << "Connected: (" << hitA->getX() << "," << hitA->getY() << "," << hitA->getZ() << ")-->("
                                          << hitB->getX() << "," << hitB->getY() << "," << hitB->getZ() << ")\n";
                  
                  map_hitFront_hitsBack[ hitA ].push_back( hitB );
                  nConnections++;
                  
               }
               
            }
            
         } 
         
      }
     
   }
   
   streamlog_out( DEBUG3 ) << "Connected " << map_hitFront_hitsBack.size() << " hits with " << nConnections << " possible overlapping hits\n";
   
   
   return map_hitFront_hitsBack;
   
   
   
}





