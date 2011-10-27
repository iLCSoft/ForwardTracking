#ifndef Criteria_h
#define Criteria_h


/**
 * Information about all Criteria.
 * 
 * For example bundles the includes.
 * 
 * Author: Robin Glattauer
 */

#include "Crit2_RZRatio.h"
#include "Crit2_StraightTrackRatio.h"
#include "Crit2_DeltaPhi.h"
#include "Crit2_HelixWithIP.h"
#include "Crit2_DeltaRho.h"

#include "Crit3_ChangeRZRatio.h"  
#include "Crit3_PT.h"
#include "Crit3_2DAngle.h"
#include "Crit3_3DAngle.h"
#include "Crit3_IPCircleDist.h"  

#include "Crit4_2DAngleChange.h"    
#include "Crit4_3DAngleChange.h" 
#include "Crit4_DistToExtrapolation.h"  
#include "Crit4_PhiZRatioChange.h"
#include "Crit4_DistOfCircleCenters.h"
#include "Crit4_NoZigZag.h"
#include "Crit4_RChange.h"




#include <map>
#include <vector>
#include <string>


namespace FTrack{

   
   
   class Criteria {
      
      
      
   public:
      
      /** @return a vector of strings that represent all types of criteria stored.
       * For example: "2Hit_Criteria" or else
       */
      static std::set< std::string > getTypes();
      
            
      /** @return a vector of all criteria of a certain type
       */
      static std::set< std::string > getCriteriaNames( std::string type );
      
      /** @return all criteria
       */
      static std::set< std::string > getAllCriteriaNames(){ return _critNames; }
      
      static std::vector< std::string > getAllCriteriaNamesVec();
      
      /** Needs to be calles before the class can be used.
       */
      static void init();
      
      /**
       * Creates a criterion with the name and the min and max values
       */
      static ICriterion* createCriterion( std::string critName , float min=0. , float max=0. )throw (UnknownCriterion) ;
      
   private:
      
      static std::set< std::string > _critNames;
      
   };

}

#endif

