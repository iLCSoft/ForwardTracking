#include "TrackPickProcessor.h"
#include <iostream>

#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>

// ----- include for verbosity dependend logging ---------
#include "marlin/VerbosityLevels.h"

#include <EVENT/LCRelation.h>
#include <IMPL/LCCollectionVec.h>
#include <EVENT/Track.h>
#include <EVENT/MCParticle.h>
#include <cmath>
#include "TVector3.h"

using namespace lcio ;
using namespace marlin ;





TrackPickProcessor aTrackPickProcessor ;


TrackPickProcessor::TrackPickProcessor() : Processor("TrackPickProcessor") {

    // modify processor description
   _description = "TrackPickProcessor makes a collection of only one / a few true tracks" ;


    // register steering parameters: name, description, class-variable, default value

   
   
   registerInputCollection(LCIO::LCRELATION,
                           "MCTrueTrackRelCollectionName",
                           "Name of the TrueTrack MC Relation collection",
                           _colNameMCTrueTracksRel,
                           std::string("TrueTracksMCP"));
   
   registerOutputCollection(LCIO::TRACKERHIT,
                            "TrackPickCollection",
                            "Name of picked track hits output collection",
                            _TrackPickCollection,
                            std::string("PickedTrackHits"));
   

}



void TrackPickProcessor::init() { 

    streamlog_out(DEBUG) << "   init called  " << std::endl ;

    // usually a good idea to
    printParameters() ;

    _nRun = 0 ;
    _nEvt = 0 ;

}


void TrackPickProcessor::processRunHeader( LCRunHeader* ) { 

    _nRun++ ;
} 



void TrackPickProcessor::processEvent( LCEvent * evt ) { 


   std::vector <TrackerHit*> hits;
   
   
  LCCollection* col = evt->getCollection( _colNameMCTrueTracksRel ) ;
   
   int nMCTracks = col->getNumberOfElements();
   
   
   // fill the vector with the relations
   for( int i=0; i < nMCTracks; i++){
      
      if ( (i >= 0)&&(i <= 19) ){
         
         
         LCRelation* rel = dynamic_cast <LCRelation*> (col->getElementAt(i) );
//          MCParticle* mcp = dynamic_cast <MCParticle*> (rel->getTo());
         Track* track    = dynamic_cast <Track*> (rel->getFrom());
         
//          std::cout << std::endl << std::endl;
//          std::cout << "Track Pick Processor picked track number " << i << " which has a px of: " << mcp->getMomentum()[2];
//          std::cout << std::endl << std::endl;
         
         std::vector <TrackerHit*> newHits = track->getTrackerHits();
         
         for( unsigned int j=0; j < newHits.size(); j++ ){ 
            
            hits.push_back( newHits[j] );
            
         }
         
      }
   
      
   }
   
   
   /*/////////////////////////////////////////////////////////////
   //temp
   LCCollection* col = evt->getCollection( "FTDTrackerHits" ) ;
   
   int nHits = col->getNumberOfElements();
   
   
   // fill the vector with the relations
   for( int i=0; i < nHits; i++)
      if ( i%2 != 0) hits.push_back( dynamic_cast <TrackerHit*> ( col->getElementAt(i) ) );
      
   // temp end   
   //////////////////////////////////////////////////////////////// */
   
   
   //finally: save the tracks
   LCCollectionVec * trkCol = new LCCollectionVec(LCIO::TRACKERHIT);
   for (unsigned int i=0; i < hits.size(); i++) trkCol->addElement( hits[i] );
   evt->addCollection(trkCol,_TrackPickCollection.c_str());    
   


   
    //-- note: this will not be printed if compiled w/o MARLINDEBUG=1 !

    streamlog_out(DEBUG) << "   processing event: " << evt->getEventNumber() 
        << "   in run:  " << evt->getRunNumber() << std::endl ;



    _nEvt ++ ;
}



void TrackPickProcessor::check( LCEvent * ) { 
    // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void TrackPickProcessor::end(){ 

    //   std::cout << "MyProcessor::end()  " << name() 
    // 	    << " processed " << _nEvt << " events in " << _nRun << " runs "
    // 	    << std::endl ;

}

