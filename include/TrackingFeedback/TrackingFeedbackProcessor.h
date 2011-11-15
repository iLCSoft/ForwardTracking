#ifndef TrackingFeedbackProcessor_h
#define TrackingFeedbackProcessor_h 1

#include "marlin/Processor.h"
#include "lcio.h"
#include <string>
#include "ICriterion.h"

#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>
#include <EVENT/LCRelation.h>
#include <EVENT/Track.h>

using namespace lcio ;
using namespace marlin ;
using namespace FTrack;



const char* TRACK_TYPE_NAMES[] = {"COMPLETE" , "COMPLETE_PLUS" , "INCOMPLETE" , "INCOMPLETE_PLUS" , "GHOST" , "LOST"}; 

/** COMPLETE: a track containing all hits of a true track, and has no further hits.
 * COMPLETE_PLUS: a track containing all hits of a true track and additional hits not belonging to the true track
 * INCOMPLETE: every hit of the track belongs to a true track, but some hits are missing
 * INCOMPLETE_PLUS: more than half of the hits of the track belong to a true track, but there are a) still true hits
 * missing in the track and b) there are additional hits not belonging to the true track.
 * (to avoid ambiguity, to which true track such a track belongs, a track should belong to a true track if the true
 * track holds more than 50% of the track.)
 * GHOST: a track that does not correspond to a true track.
 * LOST: a true track, that is not found
 */
enum TrackType { COMPLETE , COMPLETE_PLUS , INCOMPLETE , INCOMPLETE_PLUS , GHOST , LOST };

/** A struct to help with categorising tracks, or better said: relations of true tracks.
 * At the core of the struct is an LCRelation which probably links a track to a monte carlo particle.
 * In this processor this will be used to store the relations of the real tracks.
 * 
 * @param lcRelation the LCRelation* corresponding to the true track.
 * 
 * @param isLost a bool telling, if the track is lost. Set to true at construction
 * 
 * @param isFoundCompletely a bool telling, if the track is reconstructed completely (every hit is found).
 * Is set false at construction
 * 
 * @param relatedTracks a map, which stores all the tracks that are associated with the true track. (In an ideal world
 * it would be only 1: the one matching the true one.) This is the key of the map, the value is an enum, that tells
 * the type of the track.
 */
struct MyRelation{
   
   LCRelation* lcRelation;
   bool isLost;
   bool isFoundCompletely;
   std::map<Track*,TrackType> relatedTracks;
   
   MyRelation( LCRelation* rel ){isLost = true; isFoundCompletely = false; relatedTracks.clear(); lcRelation = rel;};
};


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

   std::string _rootFileName;
   std::string _treeName;

   int _nRun ;
   int _nEvt ;
   
   double _ptMin;
   double _distToIPMax;
   double _chi2ProbCut;
   int _nHitsMin;
   


   std::vector <ICriterion*> _crits2;
   std::vector <ICriterion*> _crits3;
   std::vector <ICriterion*> _crits4;

   
   bool _MSOn ;
   bool _ElossOn ;
   bool _SmoothOn ;
   
   const SectorSystemFTD* _sectorSystemFTD;
   
   
   unsigned _nComplete;         // complete tracks without extra points
   unsigned _nCompletePlus;     // complete tracks with extra points
   unsigned _nLost;             // lost tracks = tracks that do exist in reality (mcp), but are not represented in the tracksearch results  
   unsigned _nIncomplete;       // incomplete tracks without extra points. i.e.: tracks that are too short (for example 1 or 2 hits are still missing)
   unsigned _nIncompletePlus;   // incomplete tracks with extra points. the reconstructed track belongs to the true track that hold more than half of the points of the track
   unsigned _nGhost;            // ghost tracks = tracks that are reconstructed, but don't actually exist. Pure fiction. a ghost track 
   // is a track, where no real track owns more than half of the tracks hits.
   unsigned _nFoundCompletely;
   
   
   unsigned _nLostSum;
   unsigned _nGhostSum;
   unsigned _nTrueTracksSum;
   unsigned _nRecoTracksSum;
   
   std::vector< MyRelation* > _myRelations;
   
   
   void checkTheTrack( Track* track );
   double getChi2Prob( Track* track );
   double getDistToIP( MCParticle* mcp );
} ;

#endif



