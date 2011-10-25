#include "Criteria.h"

using namespace FTrack;


std::map< std::string , std::vector< std::string > > Criteria::map_type_crits;


void Criteria::init(){
   
   
   map_type_crits[ "2Hit_Criteria" ].push_back( "RZRatio" );
   map_type_crits[ "2Hit_Criteria" ].push_back( "StraightTrackRatio" );
   map_type_crits[ "2Hit_Criteria" ].push_back( "DeltaPhi" );
   map_type_crits[ "2Hit_Criteria" ].push_back( "HelixWithIP" );
   map_type_crits[ "2Hit_Criteria" ].push_back( "DeltaRho" );
   
   map_type_crits[ "3Hit_Criteria" ].push_back( "ChangeRZRatio" );
   map_type_crits[ "3Hit_Criteria" ].push_back( "PT" );
   map_type_crits[ "3Hit_Criteria" ].push_back( "2DAngle" );
   map_type_crits[ "3Hit_Criteria" ].push_back( "3DAngle" );
   map_type_crits[ "3Hit_Criteria" ].push_back( "IPCircleDist" );

   map_type_crits[ "4Hit_Criteria" ].push_back( "2DAngleChange" );
   map_type_crits[ "4Hit_Criteria" ].push_back( "3DAngleChange" );
   map_type_crits[ "4Hit_Criteria" ].push_back( "DistToExtrapolation" );
   map_type_crits[ "4Hit_Criteria" ].push_back( "PhiZRatioChange" );
   map_type_crits[ "4Hit_Criteria" ].push_back( "DistOfCircleCenters" );
   map_type_crits[ "4Hit_Criteria" ].push_back( "NoZigZag" );
   map_type_crits[ "4Hit_Criteria" ].push_back( "RChange" );


}







std::vector< std::string > Criteria::getTypes(){
 
   
   std::vector< std::string > types;
   
   std::map< std::string , std::vector< std::string > >::iterator it;
   
   
   for( it = Criteria::map_type_crits.begin(); it != Criteria::map_type_crits.end(); it++ ){
      
      
      types.push_back( it->first );
      
   }
   
   return types;
   
   
}


std::vector< std::string > Criteria::getCriteria( std::string type ){
   
   
   std::vector< std::string > criteria;
   
   
   std::map< std::string , std::vector< std::string > >::iterator it;
   
   it = map_type_crits.find( type );
   
   if( it != map_type_crits.end() ) criteria = it->second; // if there is an entry under the passed type, return the vector stored there
   
   
   return criteria;
      
         
   
 
   
   
   
}



