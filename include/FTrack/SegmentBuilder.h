#ifndef SegmentBuilder_h
#define SegmentBuilder_h

#include "FTDRepresentation.h"
#include "FTDSegRepresentation.h"
#include "ICriterion.h"
#include "IHitConnector.h"
#include "Automaton.h"

namespace FTrack{


   /** This classe builds the bridge between the Automaton and the FTDRepresentation.
    * 
    * It can be used to take all the autHits stored in an FTDRepresentation and make
    * 1-segments out of them ( see the class Segment for more info on them ).
    * 
    * The created 1-segments then are connected.
    * 
    * For the rules of connecting criteria and hitConnectors can be added to the object:
    * 
    * - a hitConnector takes a code of the segment ( for example a cellID0 or a layer number or an own code ) and returns
    * all the codes we might connect to. So there we get the information like: "this segment can be connected
    * to layer 3 and 4, module 7,8,9 inf forward direction".
    * 
    * - the criteria take two segments and return whether they are compatible. A criterion could check for anything
    * that is stored in the segments. For example: if the line formed from two 1-segments passes close by the IP might
    * be a criterion for very stiff tracks.
    * 
    * So the hitConnectors tell us were to look and the criteria whether to connect. If two 1-segments are found,
    * that compatible, they will be connected. Connected means: The inner 1-segment will save the outer one as a parent
    * and the outer on will save the inner one as a child.
    * 
    * All this (except adding hitConnectors and Criteria) is done in the method get1SegAutomaton.
    * 
    * This method finally then returns an automaton (segments sorted by their layers), ready to be used.
    * 
    */   
   class SegmentBuilder{
      
     
   public: 
      
      /**
       * @param ftdRep the FTDRepresentation to take the autHits from
       */
      SegmentBuilder(  FTDRepresentation* ftdRep);
      
      /** Adds a criterion. 
       */
      void addCriterion ( ICriterion* criterion ){ _criteria.push_back( criterion );};
      
      /** Adds criteria
       */
      void addCriteria ( std::vector< ICriterion* > criteria){ _criteria.insert( _criteria.end(), criteria.begin() , criteria.end() ); }
      
      /** Adds a hitConnector
       */
      void addHitConnector ( IHitConnector* hitConnector ){ _hitConnectors.push_back( hitConnector ); };
      
      /**
       * @return An automaton containing all the hits from the FTDRepresentation sorted now by layers. 
       * (attention: those are not necessarily the same layers as the FTD layers).
       * Also they are connected.
       */
      Automaton get1SegAutomaton();
      
      
   private:
      
      
      std::vector <ICriterion* > _criteria;
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





