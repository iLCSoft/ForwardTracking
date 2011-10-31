#ifndef MyTrack_h
#define MyTrack_h

// framework dependent
#include "IMPL/TrackImpl.h"
#include "MarlinTrk/Factory.h"
#include "MarlinTrk/IMarlinTrkSystem.h"
#include "MarlinTrk/IMarlinTrack.h"

// common 
#include <vector>

// FTrack
#include "AutHit.h"


namespace FTrack{


   class MyTrack{
      
   public:
      
      MyTrack();
      
      /** @return a track in the lcio format
       */
      Track* getLcioTrack(){ return ( new IMPL::TrackImpl(*_lcioTrack) );}
      
    
      void addHit( AutHit* hit );
      
      double getNdf(){ return _lcioTrack->getNdf(); }
      double getChi2(){ return _lcioTrack->getChi2(); }
      double getChi2Prob(){ return _chi2Prob; }
      
      std::vector< AutHit* > getHits(){ return _hits; }
      
      /** For the fitting of a track an environment (containing detector information and so on) has to be set and this
       * is done via this method. It is static, as there is only one environment and therefore only one initilisation
       * necessary for all tracks
       */
      static void initialiseFitter( const std::string& systemType,  
                                    const gear::GearMgr* mgr , 
                                    const std::string& options ,
                                    const bool MSon ,
                                    const bool ElossOn ,
                                    const bool SmoothOn );
      
      /** Fits the track and sets chi2, Ndf etc.
       */
      void fit();
      
      ~MyTrack(){ delete _lcioTrack; }
      
   protected:
      
      /** the hits the track consists of
       */
      std::vector< AutHit* > _hits;

      
      IMPL::TrackImpl* _lcioTrack;
      
      // for fitting
      static MarlinTrk::IMarlinTrkSystem* _trkSystem;
      

      double _chi2Prob;


   };





}


#endif


