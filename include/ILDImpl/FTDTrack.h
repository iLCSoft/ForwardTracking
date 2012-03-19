#ifndef FTDTrack_h
#define FTDTrack_h

// framework dependent
#include "IMPL/TrackImpl.h"
#include "MarlinTrk/Factory.h"
#include "MarlinTrk/IMarlinTrkSystem.h"
#include "MarlinTrk/IMarlinTrack.h"

// common 
#include <vector>

// FTrack
#include "IFTDHit.h"
#include "ITrack.h"


namespace FTrack{


   /** A class for ITracks containing an lcio::Track at core
    */
   class FTDTrack : public ITrack {
      
   public:
      
      FTDTrack();
      FTDTrack( std::vector< IHit* > hits );
      FTDTrack( const FTDTrack& f );
      FTDTrack & operator= (const FTDTrack & f);
      
      
      /** @return a track in the lcio format
       */
      Track* getLcioTrack(){ return ( new IMPL::TrackImpl(*_lcioTrack) );}
      
    
      void addHit( IHit* hit );
      
      virtual double getNdf() const { return _lcioTrack->getNdf(); }
      virtual double getChi2() const { return _lcioTrack->getChi2(); }
      virtual double getChi2Prob() const { return _chi2Prob; }
      
      virtual std::vector< IHit* > getHits() const 
         { std::vector<IHit*> hits; 
         for(unsigned i=0; i<_hits.size();i++) hits.push_back( _hits[i] ); 
         return hits; }
      
      virtual double getQI() const;
      
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
      virtual void fit();
      
      virtual ~FTDTrack(){ delete _lcioTrack; }
      
   protected:
      
      /** the hits the track consists of
       */
      std::vector< IFTDHit* > _hits;
      
      
      IMPL::TrackImpl* _lcioTrack;
      
      // for fitting
      static MarlinTrk::IMarlinTrkSystem* _trkSystem;
      
      
      double _chi2Prob;
      
      
   };



}


#endif


