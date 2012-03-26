#ifndef Fitter_h
#define Fitter_h

#include "MarlinTrk/Factory.h"
#include "MarlinTrk/IMarlinTrkSystem.h"
#include "MarlinTrk/IMarlinTrack.h"
#include "EVENT/Track.h"
#include "lcio.h"

#include "Math/ProbFunc.h"


using namespace lcio;



class FitterException : public std::exception {
   
   
protected:
   std::string message ;
   
   FitterException(){  /*no_op*/ ; } 
   
public: 
   virtual ~FitterException() throw() { /*no_op*/; } 
   
   FitterException( const std::string& text ){
      message = "FitterException: " + text ;
   }
   
   virtual const char* what() const  throw() { return  message.c_str() ; } 
   
};



/** A class to store additional information together with a TrackState, namely the chi2 value and Ndf
 */
class TrackStatePlus{
   
   
   
public: 
   
   const TrackState* getTrackState() const {return _trackState; }
   double getChi2() const {return _chi2;}
   int getNdf() const {return _Ndf;}
   
   TrackStatePlus( const TrackState* trackState, double chi2, int Ndf ):
      _trackState( trackState ), _chi2( chi2 ), _Ndf( Ndf ){}
   
   ~TrackStatePlus(){ delete _trackState; }
   
private: 
   
   const TrackState* _trackState;
   double _chi2;
   int _Ndf;
   
   
};


/** A class to make it quick to fit a track or hits and get back the chi2, Ndf and chi2prob values and
 * also bundle the code used for that, so it doesn't have to be copied all over the places.
 */
class Fitter{
   
   
public:
   
   Fitter( Track* track , MarlinTrk::IMarlinTrkSystem* trkSystem );
   Fitter( std::vector < TrackerHit* > trackerHits, MarlinTrk::IMarlinTrkSystem* trkSystem );
   
   double getChi2Prob( int trackStateLocation ) throw( FitterException );
   double getChi2( int trackStateLocation ) throw( FitterException );
   int getNdf( int trackStateLocation ) throw( FitterException );
   
   //TODO: maybe add methods for custom points (location: TrackState::AtOther) In that case, the point would have to
   // be passed as well. (or only the point is passed)
   
   const TrackState* getTrackState( int trackStateLocation ) throw( FitterException );
   
   ~Fitter(){ 
      
      for( unsigned i=0; i<_trackStatesPlus.size(); i++ ) delete _trackStatesPlus[i];
      _trackStatesPlus.clear();
      
      delete _marlinTrk;
      
   }
   
private:
   
   const TrackStatePlus* getTrackStatePlus( int trackStateLocation ) throw( FitterException );

   void fit()throw( FitterException );
   
   static float _bField;
   
   
   std::vector< TrackerHit* > _trackerHits;
   
   /** here the created TrackStates (plus) are stored */
   std::vector< const TrackStatePlus* > _trackStatesPlus;
   
   MarlinTrk::IMarlinTrkSystem* _trkSystem;
   
   MarlinTrk::IMarlinTrack* _marlinTrk;
   
};

#endif
