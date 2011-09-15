#ifndef SegmentBuilder_h
#define SegmentBuilder_h

#include "FTDRepresentation.h"
#include "FTDSegRepresentation.h"
#include "ICriteria.h"
#include "IHitConnector.h"
#include "Automaton.h"

namespace FTrack{


   //TODO: make this more flexible: allow to load rules for connecting hits (like: it is okay to connect from layer 5 to 4, but
   //only if the petals are close or something like this.
   
   class SegmentBuilder{
      
     
   public: 
      
      SegmentBuilder(  FTDRepresentation* ftdRep);
      
      /** adds a criteria
       */
      void addCriteria ( ICriteria* criteria ){ _criteria.push_back( criteria );};
      
      void addHitConnector ( IHitConnector* hitConnector ){ _hitConnectors.push_back( hitConnector ); };
      
      /**
       * @return all hits as 1 segments having stored their potential neighbors in a vector[ layer ][hitnr].
       */
      Automaton get1SegAutomaton();
      
      
   private:
      
      
      std::vector <ICriteria* > _criteria;
      std::vector <IHitConnector* > _hitConnectors;
      
      FTDRepresentation* _FTDRep;
      
      /** Connects two segments, if they fulfill all the criteria
       * 
       * @return whether they were connected
       */
      bool connectSegments ( Segment* parent , Segment* child );
      
      
      
      
   };





}


#endif