#ifndef StepAnalyser_h
#define StepAnalyser_h 1


#include <string>

#include "marlin/Processor.h"
#include "lcio.h"
#include <EVENT/TrackerHit.h>
#include <EVENT/Track.h>

#include "KiTrack/Segment.h"




using namespace lcio ;
using namespace marlin ;




/**  Processor to analyse the steps a true track made on its way through the FTD.
 * 
 * 
 *  <h4>Input - Prerequisites</h4>
 *  A collection of cheated tracks in the FTD.
 *
 *  <h4>Output</h4> 
 *  A root file
 * 
 * @param MCTrueTrackRelCollectionName The collection of the cheated track relations.
 * 
 * @author R. Glattauer HEPHY, Wien
 *
 */



class StepAnalyser : public Processor {
   
public:
   
   virtual Processor*  newProcessor() { return new StepAnalyser ; }
   
   
   StepAnalyser() ;
   
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
   
   
   

   
   
   
   int _nRun ;
   int _nEvt ;
   
   
   double _Bz; //B field in z direction
   
   std::string _rootFileName;
   std::string _treeName;
   std::string _treeName2;
   
   std::string _colNameMCTrueTracksRel;
   
  
   
} ;

#endif



