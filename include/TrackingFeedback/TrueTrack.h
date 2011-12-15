#ifndef TrueTrack_h
#define TrueTrack_h


#include <EVENT/MCParticle.h>
#include <EVENT/Track.h>
#include "lcio.h"

using namespace lcio;

static const char* TRACK_TYPE_NAMES[] = {"COMPLETE" , "COMPLETE_PLUS" , "INCOMPLETE" , "INCOMPLETE_PLUS" , "GHOST" , "LOST"}; 

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

//TODO: this is still all very unclean and bad encapsulated etc.

/** A class to help with categorising tracks.
 * It consists of a true track and the corresponding Monte Carlo Particle and can store
 * related Tracks. 
 */ 
class TrueTrack{
   
public:
   
   /** whether this true track is lost */  
   bool isLost;
   
   /** whether this true track was found completely (complete or complete_plus) */
   bool isFoundCompletely;
   
   /** whether a complete version (with no additional hits) was found */
   bool completeVersionExists;
   
   /** Here all the found tracks related to the true track are stored.
    * The second value, the TrackType tells, which kind of track it is
    */
   std::map<Track*,TrackType> map_track_type;
   
   TrueTrack( Track* trueTrack , MCParticle* mcp ):_trueTrack(trueTrack), _mcp(mcp) {isLost = true; isFoundCompletely = false; completeVersionExists = false;}
   
   /** Info about the Monte Carlo Particle */
   std::string getMCPInfo();
   
   /** Info about the true Track */
   std::string getTrueTrackInfo();
   
   /** Info about all the tracks associated to the true track */
   std::string getRelatedTracksInfo();
   
   /** Info about the status of the track concerning if it was found or lost */
   std::string getFoundInfo();
   
   /** @return a string containing information about the CellID of a hit
    */
   std::string cellIDInfo( TrackerHit* hit );
   
   std::string positionInfo( TrackerHit* hit );
   
   
   const Track* getTrueTrack(){ return _trueTrack; }
   
private:
   
   Track* _trueTrack;
   MCParticle* _mcp;
   
   
};


#endif