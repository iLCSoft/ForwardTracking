#include "SegmentBuilder.h"

#include "FTDSegRepresentation.h"



// ----- include for verbosity dependend logging ---------
#include "marlin/VerbosityLevels.h"

using namespace FTrack;

SegmentBuilder::SegmentBuilder(  FTDRepresentation* ftdRep){
   
   
   _FTDRep = ftdRep;
   
   
}


Automaton SegmentBuilder::get1SegAutomaton(){
   
   
 
   /**********************************************************************************************/
   /*                                                                                            */
   /*                Set up an FTDSegRrepresentation                                             */
   /*                                                                                            */
   /**********************************************************************************************/
   
   // Construct it with the same AutCode as the FTDRepresentation has
   FTDSegRepresentation ftdSegRep ( _FTDRep->getAutCode() );
   
   
   // get all the codes used in the FTDRepresentation
   std::set <int> codes = _FTDRep->getCodes();
   
      
   unsigned nCreatedSegments=0;
   
   for ( std::set<int>::iterator it = codes.begin(); it!=codes.end(); it++ ){ //over all codes

      // All the autHits with the code
      std::vector <AutHit*> autHits = _FTDRep->getHitsWithCode( *it );
   
      for ( unsigned int i=0; i<autHits.size(); i++ ){ //over every hit with this code


         // create a Segment
         Segment* segment = new Segment( autHits[i] );
         
         // Add the segment to the FTDSegRepresentation
         ftdSegRep.addSegment ( segment , *it );        // *it == the code
         
         nCreatedSegments++;
         
      }
      
   }
      
     
   streamlog_out(DEBUG4) << "\n Number of created 1-segments: " << nCreatedSegments <<"\n";


   

   /**********************************************************************************************/
   /*                                                                                            */
   /*                Now check all 1-Segments and connect them to others  4                      */
   /*                And store them in an Automaton                                              */
   /*                                                                                            */
   /**********************************************************************************************/
   
     
   unsigned nConnections=0;
   unsigned nStoredSegments = 0;
   
   Automaton automaton;
   
   
   // get all the codes used in the FTDSegRepresentation
   codes = ftdSegRep.getCodes();
   
   AutCode* autCode = _FTDRep->getAutCode(); //Needed to get the layer of the Hit

   
   for ( std::set<int>::iterator it = codes.begin(); it!=codes.end(); it++ ){ // over all codes

      // All the segments with one certain code
      int code = *it;
      std::vector <Segment*> segments = ftdSegRep.getSegsWithCode( code );
   

      
      // Now find out, what the allowed codes to connect to are:
      std::set <int> targetCodes;
                  
      for ( unsigned i=0; i < _hitConnectors.size(); i++ ){ // over all IHitConnectors we use
      
         // get the allowed targets
         std::set <int> newTargetCodes = _hitConnectors[i]->getTargetCode( code );
         
         //insert them into our set
         targetCodes.insert( newTargetCodes.begin() , newTargetCodes.end() );
         
      }
      
      
      for ( unsigned int i=0; i< segments.size(); i++ ){ //over all segments within the code

            
         Segment* parent = segments[i]; 
         
         //set its layer
         parent->setLayer( autCode->getLayer( code ) ); //We need to set this before writing it to the automaton

      
         for ( std::set<int>::iterator itTarg = targetCodes.begin(); itTarg!=targetCodes.end(); itTarg++ ){ // over all target codes
         

            
            int targetCode = *itTarg;
            std::vector <Segment*> targetSegments = ftdSegRep.getSegsWithCode( targetCode );
     
            
            for ( unsigned int j=0; j < targetSegments.size(); j++ ){ // over all segments with the target code


               Segment* child = targetSegments[j];
               
               if ( connectSegments( parent , child ) ){ //the connection was successful 
               
            
                  nConnections++;              
                  
               }                                    
               
            }
            
         }

         // Store the segment in the automaton

         
         automaton.addSegment( parent );
         nStoredSegments++;



      }      
      
   }
   
   
   streamlog_out(DEBUG4) << "\n Number of connections made " << nConnections <<"\n";

   streamlog_out( DEBUG4 ) << "\n Number of 1-segments, that got stored in the automaton: " << nStoredSegments <<"\n";
 
   
  
   
   
   return automaton;
   
   
}





bool SegmentBuilder::connectSegments ( Segment* parent , Segment* child ){


   //Check all the criteria:
   bool areCompatible = true;

   for (unsigned int iCrit = 0; iCrit < _criteria.size(); iCrit++){
      
      if ( _criteria[iCrit]->areCompatible( parent , child ) == false ){
         
         areCompatible = false;
         break;
      }
      
   }


   if ( areCompatible ) {
      
      
      parent->addChild( child );
      child->addParent( parent );
      
      return true;
      
   }
   
   
   return false;

}




