#ifndef RecoTrack_h
#define RecoTrack_h

#include "EVENT/Track.h"
#include "lcio.h"
#include "MarlinTrk/IMarlinTrkSystem.h"
#include "MarlinTrk/IMarlinTrack.h"

using namespace lcio;


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


class TrueTrack;


/** A class to make linking reconstructed tracks and true tracks easier.
 * 
 * It represents a reconstructed track, so i wrapps a Track* ( accessible by getTrack() ) and offers
 * additional functionality: 
 * 
 * It has a TrackType enum that tells what kind of reconstructed track it is. On construction it is a GHOST.
 * 
 * Also there is a vector of all assigned TrueTracks. 
 * 
 */
class RecoTrack{
   
   
public:
   
   RecoTrack( Track* track, MarlinTrk::IMarlinTrkSystem* trkSystem ): _track( track ), _trkSystem( trkSystem )
      { _type = GHOST; } 
   
   Track* getTrack(){ return _track; }
   
   
   TrackType getType() const { return _type; }
   
   void setType( TrackType type ){ _type = type; }
   
   
   /** @return the true tracks this reco track is linked to */
   std::vector< const TrueTrack* > getTrueTracks() const { return _trueTracks; }
   
   /** adds a true track */
   void addTrueTrack( TrueTrack* trueTrack ){ _trueTracks.push_back( trueTrack ); }
   
   
   
   std::string getRecoTrackInfo() const;
   
   
   /** @return a string containing information about the CellID of a hit */
   static std::string cellIDInfo( TrackerHit* hit );
   
   /** @return the position of a hit as a string */
   static std::string positionInfo( TrackerHit* hit );
   
   
   
   
protected:
   
   Track* _track;
   std::vector< const TrueTrack* > _trueTracks;
   TrackType _type;
   
   
   MarlinTrk::IMarlinTrkSystem* _trkSystem; // for fitting
   
   
   
   
   
};



#endif

