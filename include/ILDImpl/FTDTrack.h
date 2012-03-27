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

#include "Fitter.h"


namespace FTrack{


   /** A class for ITracks containing an lcio::Track at core
    */
   class FTDTrack : public ITrack {
      
   public:
      
      /** @param trkSystem An IMarlinTrkSystem, which is needed for fitting of the tracks
       */
      FTDTrack( MarlinTrk::IMarlinTrkSystem* trkSystem );
      
      /** @param hits The hits the track consists of 
       * @param trkSystem An IMarlinTrkSystem, which is needed for fitting of the tracks
       */
      FTDTrack( std::vector< IHit* > hits , MarlinTrk::IMarlinTrkSystem* trkSystem );
      FTDTrack( const FTDTrack& f );
      FTDTrack & operator= (const FTDTrack & f);
      
      
      /** @return a track in the lcio format
       */
      TrackImpl* getLcioTrack(){ return ( _lcioTrack );}
      
    
      void addHit( IHit* hit );
      
      virtual double getNdf() const { return _lcioTrack->getNdf(); }
      virtual double getChi2() const { return _lcioTrack->getChi2(); }
      virtual double getChi2Prob() const { return _chi2Prob; }
      
      virtual std::vector< IHit* > getHits() const 
         { std::vector<IHit*> hits; 
         for(unsigned i=0; i<_hits.size();i++) hits.push_back( _hits[i] ); 
         return hits; }
      
      virtual double getQI() const;
      
      
      /** Fits the track and sets chi2, Ndf etc.
       */
      virtual void fit() throw( FitterException );
      
      virtual ~FTDTrack(){ delete _lcioTrack; }
      
   protected:
      
      /** the hits the track consists of
       */
      std::vector< IFTDHit* > _hits;
      
      
      IMPL::TrackImpl* _lcioTrack;
      
      // for fitting
      MarlinTrk::IMarlinTrkSystem* _trkSystem;
      
      
      double _chi2Prob;
      
      
   };



}


#endif


