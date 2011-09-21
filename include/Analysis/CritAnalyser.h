#ifndef CritAnalyser_h
#define CritAnalyser_h 1

#include "marlin/Processor.h"
#include "lcio.h"

#include <EVENT/TrackerHit.h>
#include <EVENT/Track.h>
#include "Segment.h"
#include <EVENT/LCRelation.h>

#include "ICriterion.h"
#include <string>

using namespace lcio ;
using namespace marlin ;
using namespace FTrack;





/**  Processor to analyse what kind of criteria the hits in the FTD fulfill.
 * 
 * 
 *  <h4>Input - Prerequisites</h4>
 *  The hits in the FTDs
 *
 *  <h4>Output</h4> 
 *  A root file
 * 
 * @param FTDHitCollection The collection of the FTD hits
 * 
 * @author R. Glattauer HEPHY, Wien
 *
 */



class CritAnalyser : public Processor {
   
public:
   
   virtual Processor*  newProcessor() { return new CritAnalyser ; }
   
   
   CritAnalyser() ;
   
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
   
   
   
   /** Input collection name.
    */
   std::string _FTDHitCollection;
   
   
   
   int _nRun ;
   int _nEvt ;
   
   
   double _Bz; //B field in z direction
   
   std::string _rootFileName;
   std::string _treeName;

   std::string _colNameMCTrueTracksRel;
   
   std::vector <ICriterion*> _crits;
   std::vector <LCRelation*> _relations;
   
   LCRelation* getRelation( Segment* parent , Segment* child );
   
} ;

#endif



