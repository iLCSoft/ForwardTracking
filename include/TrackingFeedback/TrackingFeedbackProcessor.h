#ifndef TrackingFeedbackProcessor_h
#define TrackingFeedbackProcessor_h 1

#include "marlin/Processor.h"
#include "lcio.h"
#include <string>
#include "TrueTrack.h"

#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>
#include <EVENT/LCRelation.h>
#include <EVENT/Track.h>

using namespace lcio ;
using namespace marlin ;



class TrackingFeedbackProcessor : public Processor {
  
 public:
  
    virtual Processor*  newProcessor() { return new TrackingFeedbackProcessor ; }
  
  
    TrackingFeedbackProcessor() ;
  
  /** Called at the begin of the job before anything is read.
   * Use to initialize the processor, e.g. book histograms.
   */
  virtual void init() ;
  
  /** Called for every run.
   */
  virtual void processRunHeader( LCRunHeader* run ) ;
  
  /** Called for every event - the working horse.
   */
  virtual void processEvent( LCEvent * evt ) ; 
  
  
  virtual void check( LCEvent * evt ) ; 
  
  
  /** Called after data processing for clean up.
  */
  virtual void end() ;
  
  
 protected:

   /** Input collection name.
   */
   std::string _colNameMCTrueTracksRel;
   std::string _TrackCollection;

   
   std::string _tableFileName;


   int _nRun ;
   int _nEvt ;
   
   double _ptMin;
   double _distToIPMax;
   double _chi2ProbCut;
   int _nHitsMin;
   

   
   bool _MSOn ;
   bool _ElossOn ;
   bool _SmoothOn ;
   
   
   
   unsigned _nComplete;            // complete tracks without extra points
   unsigned _nCompletePlus;        // complete tracks with extra points
   unsigned _nLost;                // lost tracks = tracks that do exist in reality (mcp), but are not represented in the tracksearch results  
   unsigned _nIncomplete;          // incomplete tracks without extra points. i.e.: tracks that are too short (for example 1 or 2 hits are still missing)
   unsigned _nIncompletePlus;      // incomplete tracks with extra points. the reconstructed track belongs to the true track that hold more than half of the points of the track
   unsigned _nGhost;               // ghost tracks = tracks that are reconstructed, but don't actually exist. Pure fiction. a ghost track 
                                   // is a track, where no real track owns more than half of the tracks hits.
   unsigned _nFoundCompletely;     // tracks have been found that contain all the hits of this track
   unsigned _nTrueTracks;          // the number of true tracks from the track cheater, that where used
   unsigned _nRecoTracks;          // the number of reconstructed tracks, that were compared to the true tracks
   unsigned _nDismissedTrueTracks; // number of the true tracks, that haven't been used
   
   unsigned _nComplete_Sum;            
   unsigned _nCompletePlus_Sum;       
   unsigned _nLost_Sum;               
   unsigned _nIncomplete_Sum;         
   unsigned _nIncompletePlus_Sum;   
   unsigned _nGhost_Sum;            
   unsigned _nFoundCompletely_Sum;     
   unsigned _nTrueTracks_Sum;          
   unsigned _nRecoTracks_Sum;          
   unsigned _nDismissedTrueTracks_Sum; 
   
   
   std::vector< TrueTrack* > _trueTracks;
   
   bool _drawMCPTracks;
   bool _saveAllEventsSummary;
   std::string _summaryFileName;
  
   
   
   void checkTheTrack( Track* track );
   TrueTrack* getAssignedTrueTrack( std::vector<TrueTrack*> relatedTrueTracks , unsigned& nHitsFromAssignedTrueTrack );
      
   
   double getDistToIP( MCParticle* mcp );
   
} ;

#endif



