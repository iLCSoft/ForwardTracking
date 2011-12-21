#include "ForwardTracking00.h"
#include <algorithm>

#include <EVENT/TrackerHit.h>
#include <EVENT/Track.h>
#include <EVENT/LCCollection.h>
#include <IMPL/LCCollectionVec.h>


// ----- include for verbosity dependend logging ---------
#include "marlin/VerbosityLevels.h"

#include <MarlinCED.h>

#include <gear/GEAR.h>
#include <gear/GearParameters.h>
#include <gear/BField.h>
#include "gear/FTDParameters.h"
#include "gear/FTDLayerLayout.h"


//--------------------------------------------------------------
//My own classes begin

#include "FTDTrack.h"

#include "FTrackTools.h"
#include "TrackSubsetHopfieldNN.h"

#include "SegmentBuilder.h"
#include "Automaton.h"

#include "FTDHit00.h"




// the hit connectors
#include "FTDHitCon00.h"


//My own classe end
//--------------------------------------------------------------


using namespace lcio ;
using namespace marlin ;
using namespace FTrack;
using namespace MarlinTrk ;





ForwardTracking00 aForwardTracking00 ;


ForwardTracking00::ForwardTracking00() : Processor("ForwardTracking00") {

   // modify processor description
   _description = "ForwardTracking00 tests the Cellular Automaton" ;


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
                              "The chi2 value below which tracks will be cut",
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
   // Test
   

   
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




void ForwardTracking00::init() { 

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
   unsigned int nSensors = 1; // there is at the moment only one sensor, namely sensor 1, but as usually things start with 0...

   streamlog_out( MESSAGE ) << "  ForwardTracking00 - Use Loi style FTDParameters" << std::endl ;
   
   const gear::GearParameters& pFTD = Global::GEAR->getGearParameters("FTD");
   
   nLayers = pFTD.getDoubleVals( "FTDZCoordinate" ).size() + 1;
   

   streamlog_out( DEBUG4 ) << "using " << nLayers - 1 << " layers, " << nModules << " petals and " << nSensors << " sensors.\n";

   _Bz = Global::GEAR->getBField().at( gear::Vector3D(0., 0., 0.) ).z();    //The B field in z direction
  
 

   _sectorSystemFTD = new SectorSystemFTD( nLayers, nModules , nSensors );


   //Initialise the TrackFitter of the tracks:
   FTDTrack::initialiseFitter( "KalTest" , marlin::Global::GEAR , "" , _MSOn , _ElossOn , _SmoothOn  );



   // store the criteria where they belong
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


void ForwardTracking00::processRunHeader( LCRunHeader* run) { 

    _nRun++ ;
} 



void ForwardTracking00::processEvent( LCEvent * evt ) { 


//--CED---------------------------------------------------------------------
// Reset drawing buffer and START drawing collection

   if( _useCED ){
      
      MarlinCED::newEvent(this , 0) ; 
      
      CEDPickingHandler &pHandler=CEDPickingHandler::getInstance();
      
      pHandler.update(evt); 
   
   }

//-----------------------------------------------------------------------
  
  //get FTD geometry info
  if( _useCED ){
      const gear::GearParameters& paramFTD = Global::GEAR->getGearParameters("FTD");
      drawFTDSensors( paramFTD , 16 , 2 ); //TODO use the same values here as for the code
  }

   
  

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
      
      
      streamlog_out( MESSAGE0 ) << "\n\nNumber of hits on the FTDs: " << nHits <<"\n";

      
      // A map to store the hits according to their sectors
      std::map< int , std::vector< IHit* > > map_sector_hits;
      
      
      std::vector< IHit* > hitsTBD; //Hits to be deleted at the end
      
      for(unsigned i=0; i< nHits ; i++){
         
         
         TrackerHit* trkHit = dynamic_cast<TrackerHit*>( col->getElementAt( i ) );
         
         //Make an FTDHit00 from the TrackerHit 
         FTDHit00* ftdHit = new FTDHit00 ( trkHit , _sectorSystemFTD );
         hitsTBD.push_back(ftdHit);
       
         map_sector_hits[ ftdHit->getSector() ].push_back( ftdHit );         
         
        
      }
      
      
      IHit* virtualIPHitForward = createVirtualIPHit(1 , _sectorSystemFTD );
      hitsTBD.push_back( virtualIPHitForward );
      map_sector_hits[ virtualIPHitForward->getSector() ].push_back( virtualIPHitForward );
      
      IHit* virtualIPHitBackward = createVirtualIPHit(-1 , _sectorSystemFTD );
      hitsTBD.push_back( virtualIPHitBackward );
      map_sector_hits[ virtualIPHitBackward->getSector() ].push_back( virtualIPHitBackward );
      
      
      
      /**********************************************************************************************/
      /*                Build the segments                                                          */
      /**********************************************************************************************/
      
      
      streamlog_out( MESSAGE0 ) << "\n--SegementBuilder--" ;
      
      //Create a segmentbuilder
      SegmentBuilder segBuilder( map_sector_hits );
      
      
      
      segBuilder.addCriteria ( _crit2Vec );
      
      //Also load hit connectors
      FTDHitCon00 hitCon( _sectorSystemFTD );
      
      
      segBuilder.addHitConnector ( & hitCon );
      
      
      // And get out the 1-segments 
      Automaton automaton = segBuilder.get1SegAutomaton();
      
      
      /**********************************************************************************************/
      /*                Automaton                                                                   */
      /**********************************************************************************************/
      
      
      
      streamlog_out( MESSAGE0 ) << "\n--Automaton--" ;
      
      if( _useCED ) automaton.drawSegments();
      
      
      automaton.clearCriteria();
      automaton.addCriteria( _crit3Vec );  
      
      
      // Let the automaton lengthen its 1-segments to 2-segments
      // Because for 1-segments (== single hits) and automaton isn't very useful. TODO: verify this hyphothesis
      automaton.lengthenSegments();
      
      
      
      /*******************************/
      /*      2-hit segments         */
      /*******************************/
      
      
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
      
      
      
      
      
      
      
      
      //Get the track candidates
      std::vector < std::vector< IHit* > > autTracks = automaton.getTracks(); //TODO: the method should have a different name
      std::vector <ITrack*> autTrackCandidates;
      
      for( unsigned i=0; i < autTracks.size(); i++) autTrackCandidates.push_back( new FTDTrack( autTracks[i] ) );
      
//       for( unsigned i=0; i < autTrackCandidates.size(); i++ ) drawTrack( autTrackCandidates[i] , 0x00ffff , 3 );
//       std::vector< ITrack* > tracks = autTrackCandidates;
      
      /**********************************************************************************************/
      /*                Fitting and erasing bad fits                                                */
      /**********************************************************************************************/
      
      std::vector< ITrack* > trackCandidates;
      
      unsigned nTracksRejected = 0;
      unsigned nTracksKept = 0;
      
      for ( unsigned i=0; i < autTrackCandidates.size(); i++ ){
       
         ITrack* track = autTrackCandidates[i];
         
         
         // fit the track
         track->fit();
         
         
         streamlog_out( DEBUG2 ) << "\n Track " << i 
                                 << " chi2Prob = " << track->getChi2Prob() 
                                 << "( chi2=" << track->getChi2() 
                                 <<", Ndf=" << track->getNdf() << " )";
       
         
         if ( track->getChi2Prob() > _chi2ProbCut ){ //chi2 prob is okay
            
            
            trackCandidates.push_back( track );         
            nTracksKept++;
            
         }
         else{ // chi2 prob is too low
            
            
            nTracksRejected++;
            delete track; //Not needed anymore --> therefore delete it
            
         }
         
         
      }      
   
   
      streamlog_out (MESSAGE0)   << "\n Kept " <<  nTracksKept 
                                 << " tracks with good chi2Prob, and rejected " << nTracksRejected << "\n";
      
    
      
      
      
      /**********************************************************************************************/
      /*               Get the best subset of tracks                                                */
      /**********************************************************************************************/
     
      
      // Make a TrackSubset
      TrackSubsetHopfieldNN subset;
      subset.addTracks( trackCandidates ); 
      
      //Calculate the best subset:
      subset.calculateBestSet();
      
      std::vector< ITrack* > tracks = subset.getAcceptedTracks();
      std::vector< ITrack* > rejectedTracks = subset.getRejectedTracks();
//       for( unsigned i=0; i < tracks.size(); i++ ) drawTrack( tracks[i], 0x00ff00 , 2 );
      
//       ///////////////////
//       std::vector< ITrack* > incompatibleTracks = subset.getIncompatilbeTracks();      
//       std::vector< ITrack* > compatibleTracks = subset.getCompatilbeTracks();    
//       
//       for( unsigned i=0; i < incompatibleTracks.size(); i++ ) drawTrack( incompatibleTracks[i], 0xff8888 , 1 );
//       for( unsigned i=0; i < compatibleTracks.size(); i++ ) drawTrack( compatibleTracks[i], 0x00F5FF , 1 );
      ////////////////////
      
      // immediately delete the rejected ones
      for ( unsigned i=0; i<rejectedTracks.size(); i++){
         
         delete rejectedTracks[i];
         
      }
      
      
      
      /**********************************************************************************************/
      /*               finally: save the tracks                                                     */
      /**********************************************************************************************/
      
      LCCollectionVec * trkCol = new LCCollectionVec(LCIO::TRACK);
      for (unsigned int i=0; i < tracks.size(); i++){
         
         FTDTrack* myTrack = dynamic_cast< FTDTrack* >( tracks[i] );
         
         if( myTrack != NULL ) trkCol->addElement( myTrack->getLcioTrack() );
         
         
      }
      evt->addCollection(trkCol,_ForwardTrackCollection.c_str());
      
      
      
      streamlog_out (MESSAGE0) << "\n\n Forward Tracking found and saved " << tracks.size() << " tracks.\n\n"; 
      
      
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





void ForwardTracking00::check( LCEvent * evt ) {}


void ForwardTracking00::end(){
   
   for ( unsigned i=0; i< _crit2Vec.size(); i++) delete _crit2Vec[i];
   for ( unsigned i=0; i< _crit3Vec.size(); i++) delete _crit3Vec[i];
   for ( unsigned i=0; i< _crit4Vec.size(); i++) delete _crit4Vec[i];
   _crit2Vec.clear();
   _crit3Vec.clear();
   _crit4Vec.clear();
   
   delete _sectorSystemFTD;
   _sectorSystemFTD = NULL;
   
}

void ForwardTracking00::drawFTDSensors ( const gear::GearParameters& paramFTD , unsigned nPetalsPerDisk , unsigned nSensorsPerPetal){
   

   
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

void ForwardTracking00::drawTrack( ITrack* track, unsigned color , unsigned width ){
   
   std::vector< IHit*> hits = track->getHits(); 
   
   for ( unsigned i=1; i < hits.size(); i++ ){
      
      IHit* a = hits[i-1];
      IHit* b = hits[i];
         
      ced_line_ID( a->getX() ,a->getY() , a->getZ() , b->getX() ,b->getY() , b->getZ() , 2 , width , color, 0);
   
   }

}


