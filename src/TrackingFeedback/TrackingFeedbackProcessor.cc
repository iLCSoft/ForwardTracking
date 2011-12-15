#include "TrackingFeedbackProcessor.h"


#include "marlin/VerbosityLevels.h"


#include <cmath>
#include <fstream>
#include <algorithm>
#include <sstream>


#include <MarlinCED.h>
#include "FTrackTools.h"
#include "Fitter.h"



using namespace lcio ;
using namespace marlin ;
using namespace FTrack;




TrackingFeedbackProcessor aTrackingFeedbackProcessor ;


TrackingFeedbackProcessor::TrackingFeedbackProcessor() : Processor("TrackingFeedbackProcessor") {

    // modify processor description
   _description = "TrackingFeedbackProcessor gives feedback about the Track Search" ;


    // register steering parameters: name, description, class-variable, default value

   registerInputCollection(LCIO::TRACK,
                           "TrackCollection",
                           "Name of Track collection to check",
                           _TrackCollection,
                           std::string("ForwardTracks")); 
   
   
   registerInputCollection(LCIO::LCRELATION,
                           "MCTrueTrackRelCollectionName",
                           "Name of the TrueTrack MC Relation collection",
                           _colNameMCTrueTracksRel,
                           std::string("TrueTracksMCP"));
   
   
   registerProcessorParameter("TableFileName",
                              "Name of the table file for saving the results ",
                              _tableFileName,
                              std::string("TrackingFeedback.csv") );   
   
   registerProcessorParameter("PtMin",
                              "The minimum transversal momentum pt above which tracks are of interest in GeV ",
                              _ptMin,
                              double (0.2)  );   
   
   registerProcessorParameter("DistToIPMax",
                              "The maximum distance from the origin of the MCP to the IP (0,0,0)",
                              _distToIPMax,
                              double (250. ) );   
   
   registerProcessorParameter("Chi2ProbCut",
                              "Tracks with a chi2 probability below this value won't be considered",
                              _chi2ProbCut,
                              double (0.005) ); 
   
   registerProcessorParameter("NumberOfHitsMin",
                              "The minimum number of hits a track must have",
                              _nHitsMin,
                              int (4)  );   
   
   
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
   
   registerProcessorParameter("DrawMCPTracks",
                              "Draw the helices of the MCP (values at IP) in CED ",
                              _drawMCPTracks,
                              bool(false));
   
   registerProcessorParameter("SaveAllEventsSummary",
                              "If true the results of all events are summed up and saved in the file specified under SummaryFileName ",
                              _saveAllEventsSummary,
                              bool(false));

   registerProcessorParameter("SummaryFileName",
                              "All events are summed up and saved in this file, if SaveAllEventsSummary == true",
                              _summaryFileName,
                              std::string("TrackingFeedbackSum.csv") );   
   
}



void TrackingFeedbackProcessor::init() { 

   
   streamlog_out(DEBUG) << "   init called  " << std::endl ;

   // usually a good idea to
   printParameters() ;

  

   _nRun = 0 ;
   _nEvt = 0 ;



   if ( _drawMCPTracks ) MarlinCED::init(this) ;

   _nComplete_Sum            = 0;
   _nCompletePlus_Sum        = 0; 
   _nLost_Sum                = 0;
   _nIncomplete_Sum          = 0;
   _nIncompletePlus_Sum      = 0;
   _nGhost_Sum               = 0;
   _nFoundCompletely_Sum     = 0;
   _nTrueTracks_Sum          = 0;
   _nRecoTracks_Sum          = 0;
   _nDismissedTrueTracks_Sum = 0; 
   
}


void TrackingFeedbackProcessor::processRunHeader( LCRunHeader* run) { 

   _nRun++;
   
} 



void TrackingFeedbackProcessor::processEvent( LCEvent * evt ) { 


//-----------------------------------------------------------------------
// Reset drawing buffer and START drawing collection
   if ( _drawMCPTracks ){
      
      MarlinCED::newEvent(this , 0) ; 
      
      CEDPickingHandler &pHandler=CEDPickingHandler::getInstance();
      
      pHandler.update(evt); 
   }
//-----------------------------------------------------------------------



   
   _nComplete = 0;         
   _nCompletePlus = 0;     
   _nLost = 0;             
   _nIncomplete = 0;       
   _nIncompletePlus = 0;   
   _nGhost = 0;            
   _nFoundCompletely = 0;
   _nTrueTracks = 0;          
   _nRecoTracks = 0;          
   _nDismissedTrueTracks = 0; 
   
   LCCollection* col = evt->getCollection( _colNameMCTrueTracksRel ) ;
   
   int nMCTracks = col->getNumberOfElements();
   
   streamlog_out( DEBUG4 ) << "Number of MCP Track Relations: " << nMCTracks << "\n";
   
   
   /**********************************************************************************************/
   /*             Check the true tracks, if they are of interest                                 */
   /**********************************************************************************************/
   _trueTracks.clear();
   
   for( int i=0; i < nMCTracks; i++){
      
      
      
      LCRelation* rel = dynamic_cast <LCRelation*> (col->getElementAt(i) );
      MCParticle* mcp = dynamic_cast <MCParticle*> (rel->getTo() );
      Track*    track = dynamic_cast <Track*>      (rel->getFrom() );
      
      
      if ( _drawMCPTracks ) MarlinCED::drawMCParticle( mcp, true, evt, 2, 1, 0xff000, 10, 3.5 );
      
      
      double pt = sqrt( mcp->getMomentum()[0]*mcp->getMomentum()[0] + mcp->getMomentum()[1]*mcp->getMomentum()[1] );
      Fitter fitter( track );
      double chi2Prob = fitter.getChi2Prob();
      
      //Only store the good tracks
      if (( getDistToIP( mcp ) < _distToIPMax )&&                       //distance to IP
         ( pt > _ptMin )&&                                              //transversal momentum
         ((int) track->getTrackerHits().size() >= _nHitsMin )&&         //number of hits in track        
         ( chi2Prob > _chi2ProbCut )){                      //chi2 probability
        
         _trueTracks.push_back( new TrueTrack( track, mcp ) );
         
      }
      else _nDismissedTrueTracks++;
      
   }
   
   _nTrueTracks = _trueTracks.size();
   
   //The restored tracks, that we want to check for how good they are
   col = evt->getCollection( _TrackCollection ) ;
   

   /**********************************************************************************************/
   /*              Check the reconstructed tracks (to what true tracks they belong)              */
   /**********************************************************************************************/
   
   
   if( col != NULL ){
      
      _nRecoTracks = col->getNumberOfElements()  ;
      
      //check all the reconstructed tracks
      for(unsigned i=0; i< _nRecoTracks ; i++){
         
         Track* track = dynamic_cast <Track*> ( col->getElementAt(i) ); 
         checkTheTrack( track );
         
      }
      
      //check the relations for lost ones and completes
      for( unsigned int i=0; i < _trueTracks.size(); i++){
         if ( _trueTracks[i]->isLost == true ) _nLost++;
         if ( _trueTracks[i]->isFoundCompletely ==true ) _nFoundCompletely++;
      }
      
      
      /**********************************************************************************************/
      /*              Print various information on the true track and related ones                  */
      /**********************************************************************************************/
      
      streamlog_out( DEBUG4 ).precision (4);
      
      for( unsigned i=0; i < _trueTracks.size(); i++ ){
       
         
         TrueTrack* myRelation = _trueTracks[i];
         
         streamlog_out( DEBUG4 ) << "\n\nTrue Track " << i << "\n";
         std::string info = myRelation->getMCPInfo();
         streamlog_out( DEBUG4 ) << info;
         info = myRelation->getTrueTrackInfo();
         streamlog_out( DEBUG4 ) << info;
         info = myRelation->getFoundInfo();
         streamlog_out( DEBUG4 ) << info;
         info = myRelation->getRelatedTracksInfo();
         streamlog_out( DEBUG4 ) << info;
         
      }
      
      
      /**********************************************************************************************/
      /*              Print and save the summary of the feedback for this event                     */
      /**********************************************************************************************/
      
      _nComplete_Sum            += _nComplete;            
      _nCompletePlus_Sum        += _nCompletePlus;      
      _nLost_Sum                += _nLost;               
      _nIncomplete_Sum          += _nIncomplete;       
      _nIncompletePlus_Sum      += _nIncompletePlus;   
      _nGhost_Sum               += _nGhost;            
      _nFoundCompletely_Sum     += _nFoundCompletely;     
      _nTrueTracks_Sum          += _nTrueTracks;          
      _nRecoTracks_Sum          += _nRecoTracks;          
      _nDismissedTrueTracks_Sum += _nDismissedTrueTracks;
      
      //The statistics:
      
      float pLost=-1.;
      float ghostrate=-1.; 
      float efficiency = -1.;
      float pComplete=-1.;
      float pFoundCompletely=-1.;
      
      if (_nTrueTracks > 0) pLost = float(_nLost)/float(_nTrueTracks);           
      if (_nTrueTracks > 0) efficiency = 1. - pLost;
      if (_nRecoTracks > 0) ghostrate = float(_nGhost)/float(_nRecoTracks);             
      if (_nTrueTracks > 0) pComplete = float(_nComplete)/float(_nTrueTracks);    
      if (_nTrueTracks > 0) pFoundCompletely = float(_nFoundCompletely)/float(_nTrueTracks);
      
      // the data that will get stored
      std::vector< std::pair < std::string , float > > data;
      
      data.push_back( std::make_pair( "efficiency" , efficiency ) );  
      data.push_back( std::make_pair( "ghostrate" , ghostrate ) );  
      data.push_back( std::make_pair( "pLost" , pLost ) );  
      data.push_back( std::make_pair( "pComplete" , pComplete ) );  
      data.push_back( std::make_pair( "pFoundCompletely" , pFoundCompletely ) );  
      data.push_back( std::make_pair( "nComplete" , _nComplete ) );  
      data.push_back( std::make_pair( "nCompletePlus" , _nCompletePlus ) );  
      data.push_back( std::make_pair( "nLost" , _nLost ) );  
      data.push_back( std::make_pair( "nIncomplete" , _nIncomplete ) );  
      data.push_back( std::make_pair( "nIncompletePlus" , _nIncompletePlus ) );  
      data.push_back( std::make_pair( "nGhost" , _nGhost ) );  
      data.push_back( std::make_pair( "nFoundCompletely" , _nFoundCompletely ) );  
      data.push_back( std::make_pair( "nTrueTracks" , _nTrueTracks ) );  
      data.push_back( std::make_pair( "nRecoTracks" , _nRecoTracks ) );  
      data.push_back( std::make_pair( "nDismissedTrueTracks" , _nDismissedTrueTracks ) );  

      streamlog_out( MESSAGE0 ) << "\n\n";
      for( unsigned i=0; i<data.size(); i++ ) streamlog_out( MESSAGE0 ) << data[i].first << "= " << data[i].second << "\n";      
      streamlog_out( MESSAGE0 ) << "\n";


      
      std::ofstream myfile;
      myfile.open (_tableFileName.c_str() , std::ios::app);
      
      if (isFirstEvent()) myfile << "\n";
      myfile<< "\n";
      for( unsigned i=0; i<data.size(); i++ ) myfile << data[i].first << "\t" << data[i].second << "\t\t";   
      myfile.close();

      
      
 
   }
   
  
   if ( _drawMCPTracks )   MarlinCED::draw(this); 


   for( unsigned int k=0; k < _trueTracks.size(); k++) delete _trueTracks[k];
   _trueTracks.clear();

   _nEvt ++ ;
}



void TrackingFeedbackProcessor::check( LCEvent * evt ) { 
    // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void TrackingFeedbackProcessor::end(){ 


   
 
   if( _saveAllEventsSummary ){
      
      std::ofstream myfile;
      myfile.open ( _summaryFileName.c_str() , std::ios::app);
      
      
      double efficiency = double( _nTrueTracks_Sum - _nLost_Sum ) / double( _nTrueTracks_Sum );
      double ghostrate = double( _nGhost_Sum ) / double( _nRecoTracks_Sum );
      double rateOfCompletes = double( _nComplete_Sum ) / double( _nTrueTracks_Sum );
      
      myfile << "\n";
      myfile << "Efficiency\t" << efficiency << "\t\t";
      myfile << "ghostrate\t"  << ghostrate  << "\t\t";
      myfile << "rateOfCompletes\t"  << rateOfCompletes  << "\t\t";
      
      
      myfile.close();
      
   }   
   

}


double TrackingFeedbackProcessor::getDistToIP( MCParticle* mcp ){
   
   double dist = sqrt(mcp->getVertex()[0]*mcp->getVertex()[0] + 
   mcp->getVertex()[1]*mcp->getVertex()[1] + 
   mcp->getVertex()[2]*mcp->getVertex()[2] );

   return dist;
   
}

 
 
 
void TrackingFeedbackProcessor::checkTheTrack( Track* track ){ 
 
   
   std::vector <TrackerHit*> hitVec = track->getTrackerHits();

   std::vector<TrueTrack*> hitRelations; //to contain all the true tracks relations that correspond to the hits of the track
                                          //if for example a track consists of 3 points from one true track and two from another
                                          //at the end this vector will have five entries: 3 times one true track and 3 times the other.
                                          //so at the end, all we have to do is to count them.

   for( unsigned int j=0; j < hitVec.size(); j++ ){ //over all hits in the track
      
      
      for( unsigned int k=0; k < _trueTracks.size(); k++){ //check all relations if they correspond 
         
         
         const Track* trueTrack = _trueTracks[k]->getTrueTrack();
         
         if ( find (trueTrack->getTrackerHits().begin() , trueTrack->getTrackerHits().end() , hitVec[j] ) 
            !=  trueTrack->getTrackerHits().end())      // if the hit is contained in this truetrack
         hitRelations.push_back( _trueTracks[k] );     //add the track (i.e. its relation) to the vector hitRelations
      }
      
   } 


   // After those two for loops we have the vector hitRelations filled with all the true track (relations) that correspond
   // to our reconstructed track. Ideally this vector would now only consist of the same true track again and again.
   //
   // Before we can check what kind of track we have here, we have to get some data.
   // We wanna know the most represented true track:

   unsigned nHitsOneTrack = 0;              //number of most true hits corresponding to ONE true track 
   TrueTrack* dominantTrueTrack = NULL;    //the true track most represented in the track 

   sort ( hitRelations.begin() , hitRelations.end()); //Sorting, so all the same elements are at one place

   unsigned n = 0;                  // number of hits from one track
   TrueTrack* previousRel = NULL;  // used to store the previous relation, so we can compare, if there was a change. at first we don't have
   // a previous one, so we set it NULL
   for (unsigned int j=0; j< hitRelations.size(); j++){ 
      
      if ( hitRelations[j] == previousRel ) n++;   //for every repeating element count 1. (that's the reason we sorted before, so we can just count them like this)
               else n = 1; //previous was different --> we start again with 1
               
               previousRel = hitRelations[j];
      
      if (n > nHitsOneTrack){ //we have a new winner (a true track) with (currently) the most hits in this track
                              
                  nHitsOneTrack = n;
                  dominantTrueTrack = hitRelations[j];
                  
      }
      
   }

   unsigned nHitsTrack = hitVec.size();   //number of hits of the reconstructed track
   unsigned nHitsTrueTrack = 0;           //number of hits of the true track

   if (dominantTrueTrack != NULL ){ 
      const Track* trueTrack = dominantTrueTrack->getTrueTrack();
      nHitsTrueTrack = trueTrack->getTrackerHits().size();  
      
   }




   //So now we have all the values we need to know to tell, what kind of track we have here.

   if ( nHitsOneTrack <= nHitsTrack/2. ){  // less than half the points at maximum correspond to one track, this is not a real track, but
      _nGhost++;                           // a ghost track
     
   }
   else{                                   // more than half of the points form a true track!
      
      TrackType trackType;
      
      if (nHitsOneTrack > nHitsTrueTrack/2.) //If more than half of the points from the true track are covered
         dominantTrueTrack->isLost = false;  // this is guaranteed no lost track  
         
      if (nHitsOneTrack < nHitsTrueTrack){    // there are too few good hits,, something is missing --> incomplete
      
         if (nHitsOneTrack < nHitsTrack){       // besides the hits from the true track there are also additional ones-->
            _nIncompletePlus++;                 // incomplete with extra points
            trackType = INCOMPLETE_PLUS;
         }   
         else{                                   // the hits from the true track fill the entire track, so its an
            _nIncomplete++;                      // incomplete with no extra points
            trackType = INCOMPLETE;
         }
      
      }
      else{                                  // there are as many good hits as there are hits in the true track
                                             // i.e. the true track is represented entirely in this track
         dominantTrueTrack->isFoundCompletely = true;
         
         
         if (nHitsOneTrack < nHitsTrack){       // there are still additional hits stored in the track, it's a
            _nCompletePlus++;                    // complete track with extra points
            trackType = COMPLETE_PLUS;
         }
         else{                                   // there are no additional points, finally, this is the perfect
            _nComplete++;                        // complete track
            trackType= COMPLETE;
            dominantTrueTrack->completeVersionExists = true;
         }
      }
      
      //we want the true track to know all reconstructed tracks containing him
      dominantTrueTrack->map_track_type.insert( std::pair<Track*, TrackType> ( track ,trackType ) );  
      
   }   
 
 
}
