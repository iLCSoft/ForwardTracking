#include "ForwardTracking.h"
#include <algorithm>

#include <EVENT/TrackerHit.h>
#include <EVENT/Track.h>
#include <EVENT/LCCollection.h>
#include <IMPL/LCCollectionVec.h>


// ----- include for verbosity dependend logging ---------
#include "marlin/VerbosityLevels.h"

#include <MarlinCED.h>




//--------------------------------------------------------------
//My own classes begin

#include "MyTrack.h"

#include "FTrackTools.h"
#include "TrackSubset.h"

#include "SegmentBuilder.h"
#include "Automaton.h"

#include "AutHit.h"
#include "SimpleHit.h"



// the hit connectors
#include "HitCon.h"


//My own classe end
//--------------------------------------------------------------


using namespace lcio ;
using namespace marlin ;
using namespace FTrack;
using namespace MarlinTrk ;





ForwardTracking aForwardTracking ;


ForwardTracking::ForwardTracking() : Processor("ForwardTracking") {

   // modify processor description
   _description = "ForwardTracking tests the Cellular Automaton" ;


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
   

   Criteria::init();
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


   MarlinCED::init(this) ;    //CED
   


   // TODO: get this from gear
   unsigned int nLayers = 8; // layer 0 is for the IP
   unsigned int nModules = 16;
   unsigned int nSensors = 2;  

   _sectorSystemFTD = new SectorSystemFTD( nLayers, nModules , nSensors );


   //Initialise the TrackFitter of the tracks:
   MyTrack::initialiseFitter( "KalTest" , marlin::Global::GEAR , "" , _MSOn , _ElossOn , _SmoothOn  );



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


void ForwardTracking::processRunHeader( LCRunHeader* run) { 

    _nRun++ ;
} 



void ForwardTracking::processEvent( LCEvent * evt ) { 


//--CED---------------------------------------------------------------------
// Reset drawing buffer and START drawing collection

//   MarlinCED::newEvent(this , 0) ; 

//   CEDPickingHandler &pHandler=CEDPickingHandler::getInstance();

//   pHandler.update(evt); 

//-----------------------------------------------------------------------
  
  //get FTD geometry info
//    const gear::GearParameters& paramFTD = Global::GEAR->getGearParameters("FTD");
//    drawFTDSensors( paramFTD , 16 , 2 ); //TODO use the same values here as for the code


   _Bz = Global::GEAR->getBField().at( gear::Vector3D(0., 0., 0.) ).z();    //The B field in z direction
  

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
      streamlog_out( MESSAGE0 ) << "\n--FTDRepresentation--" ;
      
      // A map to store the hits according to their sectors
      std::map< int , std::vector< IHit* > > map_sector_hits;
      
      
      std::vector< IHit* > hitsTBD; //Hits to be deleted at the end
      
      for(unsigned i=0; i< nHits ; i++){
         
         
         TrackerHit* trkHit = dynamic_cast<TrackerHit*>( col->getElementAt( i ) );
         
         //Make an AutHit from the TrackerHit 
         AutHit* autHit = new AutHit ( trkHit , _sectorSystemFTD );
         hitsTBD.push_back(autHit);
       
         map_sector_hits[ autHit->getSector() ].push_back( autHit );         
         
        
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
      HitCon hitCon( _sectorSystemFTD );
      
      segBuilder.addHitConnector ( & hitCon );
      
      
      // And get out the 1-segments
      Automaton automaton = segBuilder.get1SegAutomaton();
      
      
      /**********************************************************************************************/
      /*                Automaton                                                                   */
      /**********************************************************************************************/
      
      
      
      streamlog_out( MESSAGE0 ) << "\n--Automaton--" ;
      
//       automaton.drawSegments();
      
      
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
      
      for( unsigned i=0; i < autTracks.size(); i++) autTrackCandidates.push_back( new MyTrack( autTracks[i] ) );
      
      std::vector< ITrack* > tracks = autTrackCandidates;
      
      /**********************************************************************************************/
      /*                Fitting and erasing bad fits                                                */
      /**********************************************************************************************/
      /*
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
      
    
      
      */
      
      /**********************************************************************************************/
      /*               Get the best subset of tracks                                                */
      /**********************************************************************************************/
     /*
      
      // Make a TrackSubset
      TrackSubset subset;
      subset.addTracks( trackCandidates ); 
      
      //Calculate the best subset:
      subset.calculateBestSet();
      
      std::vector< ITrack* > tracks = subset.getBestTrackSubset();
      std::vector< ITrack* > rejectedTracks = subset.getRejectedTracks();

      // immediately delete the rejected ones
      for ( unsigned i=0; i<rejectedTracks.size(); i++){
         
         delete rejectedTracks[i];
         
      }
      
      */
      
      /**********************************************************************************************/
      /*               finally: save the tracks                                                     */
      /**********************************************************************************************/
      
      LCCollectionVec * trkCol = new LCCollectionVec(LCIO::TRACK);
      for (unsigned int i=0; i < tracks.size(); i++){
         
         MyTrack* myTrack = dynamic_cast< MyTrack* >( tracks[i] );
         
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




//    MarlinCED::draw(this);

       
       
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




