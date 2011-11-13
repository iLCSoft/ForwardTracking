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
#include "ITrack.h"


namespace FTrack{


   class MyTrack : public ITrack {
      
   public:
      
      MyTrack();
      MyTrack( std::vector< IHit* > hits );
      
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
      
      virtual double getQI() const { return getChi2Prob(); }
      
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


