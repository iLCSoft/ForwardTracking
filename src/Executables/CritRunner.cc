/** Executable, that just uses the criteria and executes them as often as the passed argument
 * or 1 time by default. 
 * The sense is to use this executable together with a profiler to see how time consuming the 
 * different criteria are.
 */

#include <cstdlib>
#include <iostream>


#include "Criteria.h"
#include "SimpleHit.h"


using namespace FTrack;

void checkCrits( std::vector< ICriterion* > critVec, Segment* parent, Segment* child ){
   
   
   
   for( unsigned i=0; i<critVec.size(); i++){
      
      critVec[i]->areCompatible( parent , child );
      
   }
 
   return;
   
   
}

int main(int argc,char *argv[]){
   
   
   std::cout << "\n\nCritRunner started\n\n";
   
   
   
   /**********************************************************************************************/
   /*            Get the criteria                                                                */
   /**********************************************************************************************/
   
   std::vector< std::string > allCriteria = Criteria::getAllCriteriaNamesVec();
   
   std::vector< ICriterion* > crit2Vec;
   std::vector< ICriterion* > crit3Vec;
   std::vector< ICriterion* > crit4Vec;
   
   for( unsigned i=0; i< allCriteria.size(); i++ ){
      
      std::string critName = allCriteria[i];
      
      ICriterion* crit = Criteria::createCriterion( critName, 0. , 0. );
      
      std::string type = crit->getType();
      
      std::cout <<  "\nAdded: Criterion " << critName 
                << " (type =  " << type << " )";
      
      if( type == "2Hit" ){
         
         crit2Vec.push_back( crit );
         
      }
      else 
      if( type == "3Hit" ){
         
         crit3Vec.push_back( crit );
         
      }
      else 
      if( type == "4Hit" ){
         
         crit4Vec.push_back( crit );
         
      }
      else delete crit;
            
            
   }
   
   /**********************************************************************************************/
   /*             Make the segments, the criteria can use                                        */
   /**********************************************************************************************/
   const SectorSystemFTD sectorSystemFTD( 1 , 1 , 1 );
   
   
   int side = 1;
   unsigned layer = 0;
   unsigned module = 0;
   unsigned sensor = 0;
   
   SimpleHit hit1 ( 0.,0.,0., side , layer , module , sensor , &sectorSystemFTD );
   SimpleHit hit2 ( 1.,3.,1., side , layer , module , sensor , &sectorSystemFTD );
   SimpleHit hit3 ( 4.,7.,2., side , layer , module , sensor , &sectorSystemFTD );
   SimpleHit hit4 ( 8.,13.,3., side , layer , module , sensor , &sectorSystemFTD );
   // the values here are chosen quite randomly, they just should give something non trivial,
   // so the criteria don't stop too early because, they can't check (for example if 3 hits 
   // are on one line, a circle can't be calculated etc.
  
   
   std::vector< IHit* > hitVecParent;
   std::vector< IHit* > hitVecChild;
   
   
   hitVecParent.push_back( &hit4 );
   hitVecChild.push_back( &hit3 );
   
   Segment segment1Parent( hitVecParent );
   Segment segment1Child( hitVecChild );
   
   
   hitVecParent.push_back( &hit3 );
   hitVecChild.push_back( &hit2 );
   
   Segment segment2Parent( hitVecParent );
   Segment segment2Child( hitVecChild );   
   
   
   hitVecParent.push_back( &hit2 );
   hitVecChild.push_back( &hit1 );
   
   Segment segment3Parent( hitVecParent );
   Segment segment3Child( hitVecChild );   
   
   
   
   /**********************************************************************************************/
   /*             Use the criteria on the segments for some times                                */
   /**********************************************************************************************/
   
   
   unsigned nRuns = 1.;
   
   if( argc >= 2 ) nRuns = atoi( argv[1] );
   
   
   std::cout << "\n\nNow using the criteria for " << nRuns << " times. \n";

   
   for( unsigned i=0; i<nRuns; i++ ){
      
//       checkCrits( crit2Vec , &segment1Parent, &segment1Child );
//       checkCrits( crit3Vec , &segment2Parent, &segment2Child );
      checkCrits( crit4Vec , &segment3Parent, &segment3Child );
      
      
   }
   
   
   
   std::cout << "\nDone!\n";
   
   
   
   
   
   for ( unsigned i=0; i< crit2Vec.size(); i++) delete crit2Vec[i];
   for ( unsigned i=0; i< crit3Vec.size(); i++) delete crit3Vec[i];
   for ( unsigned i=0; i< crit4Vec.size(); i++) delete crit4Vec[i];
   
   
   return 0;
   
   
}

