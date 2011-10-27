#include "Criteria.h"

using namespace FTrack;


std::set< std::string > Criteria::_critNames;


void Criteria::init(){
   
   
   _critNames.insert( "Crit2_RZRatio" );
   _critNames.insert( "Crit2_StraightTrackRatio" );
   _critNames.insert( "Crit2_DeltaPhi" );
   _critNames.insert( "Crit2_HelixWithIP" );
   _critNames.insert( "Crit2_DeltaRho" );
   
   _critNames.insert( "Crit3_ChangeRZRatio" );
   _critNames.insert( "Crit3_PT" );
   _critNames.insert( "Crit3_2DAngle" );
   _critNames.insert( "Crit3_3DAngle" );
   _critNames.insert( "Crit3_IPCircleDist" );

   _critNames.insert( "Crit4_2DAngleChange" );
   _critNames.insert( "Crit4_3DAngleChange" );
   _critNames.insert( "Crit4_DistToExtrapolation" );
   _critNames.insert( "Crit4_PhiZRatioChange" );
   _critNames.insert( "Crit4_DistOfCircleCenters" );
   _critNames.insert( "Crit4_NoZigZag" );
   _critNames.insert( "Crit4_RChange" );


}







std::set < std::string > Criteria::getTypes(){
 
   
   std::set< std::string > types;
   
   std::set< std::string >::iterator it;
   
   
   for( it = _critNames.begin(); it != _critNames.end(); it++ ){
      
      
      ICriterion* crit = Criteria::createCriterion( *it );
      
      types.insert( crit->getType() );
      
      delete crit;
      
      
   }
   
   return types;
   
   
}


std::set< std::string > Criteria::getCriteriaNames( std::string type ){
   
   
   std::set< std::string > criteria;
   
   
   std::set< std::string >::iterator it;
   
   for( it = _critNames.begin(); it != _critNames.end(); it++ ){
      
      
      ICriterion* crit = Criteria::createCriterion( *it );
      
      if ( crit->getType() == type ) criteria.insert( *it );
      
      delete crit;
      
   }

   return criteria;
      
    
   
   
}


ICriterion* Criteria::createCriterion( std::string critName, float min , float max ) throw (UnknownCriterion){
   
   
   
   if ( critName == "Crit2_RZRatio" ) return ( new Crit2_RZRatio( min , max ) );
   
   else if ( critName == "Crit2_RZRatio" ) return ( new Crit2_RZRatio( min , max ) );
   
   else if ( critName == "Crit2_StraightTrackRatio" ) return ( new Crit2_StraightTrackRatio( min , max ) );
   
   else if ( critName == "Crit2_DeltaPhi" ) return ( new Crit2_DeltaPhi( min , max ) );
   
   else if ( critName == "Crit2_HelixWithIP" ) return ( new Crit2_HelixWithIP( min , max ) );
   
   else if ( critName == "Crit2_DeltaRho" ) return ( new Crit2_DeltaRho( min , max ) );
   
   else if ( critName == "Crit3_ChangeRZRatio" ) return ( new Crit3_ChangeRZRatio( min , max ) );
   
   else if ( critName == "Crit3_PT" ) return ( new Crit3_PT( min , max ) );
   
   else if ( critName == "Crit3_2DAngle" ) return ( new Crit3_2DAngle( min , max ) );
   
   else if ( critName == "Crit3_3DAngle" ) return ( new Crit3_3DAngle( min , max ) );
   
   else if ( critName == "Crit3_IPCircleDist" ) return ( new Crit3_IPCircleDist( min , max ) );
   
   else if ( critName == "Crit4_2DAngleChange" ) return ( new Crit4_2DAngleChange( min , max ) );
   
   else if ( critName == "Crit4_3DAngleChange" ) return ( new Crit4_3DAngleChange( min , max ) );
   
   else if ( critName == "Crit4_DistToExtrapolation" ) return ( new Crit4_DistToExtrapolation( min , max ) );
   
   else if ( critName == "Crit4_PhiZRatioChange" ) return ( new Crit4_PhiZRatioChange( min , max ) );
   
   else if ( critName == "Crit4_DistOfCircleCenters" ) return ( new Crit4_DistOfCircleCenters( min , max ) );
   
   else if ( critName == "Crit4_NoZigZag" ) return ( new Crit4_NoZigZag( min , max ) );
   
   else if ( critName == "Crit4_RChange" ) return ( new Crit4_RChange( min , max ) );
   
   
   else {
      
      std::string s = "Criteria::The criterion \"" + critName + 
                      "\" is not known. Make sure the class Criteria has this criterion listed in the createCriterion method";
      
      throw UnknownCriterion( s );
      
   }

      
      
    
}


std::vector< std::string > Criteria::getAllCriteriaNamesVec(){
   
   std::vector < std::string > allCriteriaNamesVec;
   
   std::set< std::string >::iterator it;
   
   for( it = _critNames.begin(); it != _critNames.end(); it++ ){
   
      
      allCriteriaNamesVec.push_back( *it );
      
   }
   
   return allCriteriaNamesVec;
   
}




