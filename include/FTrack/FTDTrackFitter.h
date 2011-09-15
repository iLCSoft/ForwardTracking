#ifndef FTDTrackFitter_h
#define FTDTrackFitter_h

#include "EVENT/Track.h"
#include "MarlinTrk/Factory.h"
#include "MarlinTrk/IMarlinTrkSystem.h"
#include "MarlinTrk/IMarlinTrack.h"


namespace FTrack {
   
   
   class FTDTrackFitter{
   
      
  
   public:
   
      FTDTrackFitter();
      
      
      /**returns a vector of the tracks, now fitted
       */
      std::vector < EVENT::Track* > getFittedTracks();
      
      
      /**Initialises the System. Has to be called before anything else can be done
       */      
      void initialise( const std::string& systemType, const gear::GearMgr* mgr , const std::string& options );
   
      
      void addTrack( EVENT::Track* track ){ _tracks.push_back( track ); };
      void addTracks( std::vector< EVENT::Track*> tracks ){ _tracks.insert( _tracks.end() ,  tracks.begin() , tracks.end() ); };
      void clearTracks() { _tracks.clear(); };
      
      void setMSOn( bool on ){ _MSOn = on; };
      void setElossOn( bool on ){ _ElossOn = on; };
      void setSmoothOn( bool on ){ _SmoothOn = on; };
      
      
   private:
      
      
      /** Pointer to the IMarlinTrkSystem instance.
       * Needed for the fitting of the track. 
       */
      
      std::vector < EVENT::Track* >  _tracks;
      MarlinTrk::IMarlinTrkSystem* _trkSystem ;
      
      bool _MSOn ;
      bool _ElossOn ;
      bool _SmoothOn ;
      
      
   
   
   };
   
   
}

#endif