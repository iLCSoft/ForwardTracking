#ifndef EndcapTrack_h
#define EndcapTrack_h

#include "IMPL/TrackImpl.h"
#include "MarlinTrk/IMarlinTrkSystem.h"
#include "MarlinTrk/IMarlinTrack.h"

#include <vector>

#include "IEndcapHit.h"
#include "KiTrack/ITrack.h"

#include "Tools/Fitter.h"


namespace KiTrackMarlin{


   /** A class for ITracks containing an lcio::Track at core
    */
   class EndcapTrack : public ITrack {
      
   public:
      
      /** @param trkSystem An IMarlinTrkSystem, which is needed for fitting of the tracks
       */
      EndcapTrack( MarlinTrk::IMarlinTrkSystem* trkSystem );
      
      /** @param hits The hits the track consists of 
       * @param trkSystem An IMarlinTrkSystem, which is needed for fitting of the tracks
       */
      EndcapTrack( std::vector< IEndcapHit* > hits , MarlinTrk::IMarlinTrkSystem* trkSystem );
      EndcapTrack( const EndcapTrack& f );
      EndcapTrack & operator= (const EndcapTrack & f);
      
      
      /** @return a track in the lcio format
       */
      TrackImpl* getLcioTrack(){ return ( _lcioTrack );}
      
    
      void addHit( IEndcapHit* hit );
      
      virtual double getNdf() const { return _lcioTrack->getNdf(); }
      virtual double getChi2() const { return _lcioTrack->getChi2(); }
      virtual double getChi2Prob() const { return _chi2Prob; }
      //virtual double getPT() const ;
      /*            
      virtual std::vector< IHit* > getHits() const 
         { std::vector<IHit*> hits; 
         for(unsigned i=0; i<_hits.size();i++) hits.push_back( _hits[i] ); 
         return hits; }
      */
      virtual std::vector< IHit* > getHits() const 
         { std::vector<IHit*> hits; 
         for(unsigned i=0; i<_hits.size();i++) hits.push_back( _hits[i] ); 
         return hits; }
      
      virtual double getQI() const;
      
      
      /** Fits the track and sets chi2, Ndf etc.
       */
      virtual void fit() ;
      
      virtual ~EndcapTrack(){ delete _lcioTrack; }
      

 

   protected:
      
      /** the hits the track consists of
       */
      std::vector< IEndcapHit* > _hits;
      
      IMPL::TrackImpl* _lcioTrack;
      
      // for fitting
      MarlinTrk::IMarlinTrkSystem* _trkSystem;
      
      
      double _chi2Prob;
      
      
   };



}


#endif


