#ifndef Fitter_h
#define Fitter_h

#include "MarlinTrk/Factory.h"
#include "MarlinTrk/IMarlinTrkSystem.h"
#include "MarlinTrk/IMarlinTrack.h"
#include <EVENT/Track.h>
#include "lcio.h"

#include "Math/ProbFunc.h"


using namespace lcio;


/** A class to make it quick to fit a track or hits and get back the chi2, Ndf and chi2prob values.
 */
class Fitter{
   
   
public:
   
   Fitter( Track* track );
   Fitter( std::vector < TrackerHit* > trackerHits );
   
   double getChi2Prob(){ return ROOT::Math::chisquared_cdf_c( _chi2 , _Ndf ); }
   double getChi2(){ return _chi2; }
   int getNdf(){return _Ndf; }
   
private:
   

   void fit();
   void init();
   
   std::vector < TrackerHit* > _trackerHits;
   
   
   static MarlinTrk::IMarlinTrkSystem* _trkSystem;
   
   
   double _chi2Prob;
   double _chi2;
   int _Ndf;
   
};

#endif