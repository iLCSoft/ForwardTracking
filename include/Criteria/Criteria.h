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
      static std::vector< std::string > getTypes();
      
      
      static std::vector< std::string > getCriteria( std::string type );
      
      /** Needs to be calles before the class can be used.
       */
      static void init();
      
      
   private:
      
      static std::map< std::string , std::vector< std::string > > map_type_crits;
      
   };

}

#endif

