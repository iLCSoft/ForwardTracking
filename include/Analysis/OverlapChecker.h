#ifndef OverlapChecker_h
#define OverlapChecker_h 1

#include "marlin/Processor.h"
#include "lcio.h"

#include <EVENT/TrackerHit.h>
#include <EVENT/Track.h>


#include <string>

using namespace lcio ;
using namespace marlin ;





/**  Processor to check if tracks are overlapping.
 * 
 * 
 *  <h4>Input - Prerequisites</h4>
 *  A collection of tracks.
 *
 * 
 * @param TrackCollectionName The collection of the tracks to check.
 * 
 * @author R. Glattauer HEPHY, Wien
 *
 */



class OverlapChecker : public Processor {
   
public:
   
   virtual Processor*  newProcessor() { return new OverlapChecker ; }
   
   
   OverlapChecker() ;
   
   /** Called at the begin of the job before anything is read.
    * Use to initialize the processor, e.g. book histograms.
    */
   virtual void init() ;
   
   /** Called for every run.
    */
   virtual void processRunHeader( LCRunHeader* run ) ;
   
   /** Called for every event - the working horse.
    */
   virtual void processEvent( LCEvent * evt ) ; 
   
   
   virtual void check( LCEvent * evt ) ; 
   
   
   /** Called after data processing for clean up.
    */
   virtual void end() ;
   
   
   
   
protected:
   
   
   
   bool areCompatible( Track* trackA , Track* trackB );
   
   
   
   int _nRun ;
   int _nEvt ;
   
   
   std::string _trackCollectionName;
   
  
   
} ;

#endif



