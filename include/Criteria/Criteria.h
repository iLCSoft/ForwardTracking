#ifndef Criteria_h
#define Criteria_h




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




#include <vector>
#include <set>
#include <string>


namespace FTrack{

   /**
    * Information about all Criteria.
    * 
    * For example bundles the includes.
    * 
    * Author: Robin Glattauer
    */   
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
      static std::set< std::string > getAllCriteriaNames();
      
      static std::vector< std::string > getAllCriteriaNamesVec();
      
      
      /**
       * Creates a criterion with the name and the min and max values
       */
      static ICriterion* createCriterion( std::string critName , float min=0. , float max=0. )throw (UnknownCriterion) ;
      
      /**
       * sets values for the passed referneced floats left and right. They indicate how
       * the specified criterion should be cut if necessary. Say you want for example
       * a 99% quantile, so that 99% of your true tracks are within it.
       * A criterion like the angle between two segments then needs to define a boarder like:
       * between an angle of 1° and of 9° there will be 99%.
       * So 1% is outside. But should 1% be the ones with a bigger angle or with a smaller angle or should
       * this be 50:50? 
       * This is defined by left and right. Left is the proportion, that is taken away on the left side and
       * right is the one that is taken away on the right side.
       * In the case of an angle we will most probably have lots around 0° and a long tail to the right, so
       * left = 0 and right = 1. seems like a good idea.
       * Standard is of course 0.5 and 0.5
       */
      static void getLeftRight( std::string critName, float & left, float & right );
      
   
      
   };

}

#endif

