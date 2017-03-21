#include "RecoTrack.h"


#include <sstream>
#include <cmath>
#include <algorithm>

#include "UTIL/LCTrackerConf.h"

#include "Tools/Fitter.h"
#include "Tools/KiTrackMarlinTools.h"

static const char* TRACK_TYPE_NAMES[] = {"COMPLETE" , "COMPLETE_PLUS" , "INCOMPLETE" , "INCOMPLETE_PLUS" , "GHOST" , "LOST"}; 

std::string RecoTrack::cellIDInfo( TrackerHit* hit ){
   
   
   std::stringstream info;
   
   UTIL::BitField64  cellID( LCTrackerCellID::encoding_string() );
   cellID.setValue( hit->getCellID0() );
   int subdet = cellID[ UTIL::LCTrackerCellID::subdet() ] ;
   int side   = cellID[ LCTrackerCellID::side() ];
   int layer  = cellID[ LCTrackerCellID::layer() ];
   int module = cellID[ LCTrackerCellID::module() ];
   int sensor = cellID[ LCTrackerCellID::sensor() ];
   
   info << "subdet " << subdet << ", side " << side << ", layer " << layer << ", module " << module << ", sensor " << sensor;
   
   return info.str();
   
}

std::string RecoTrack::positionInfo( TrackerHit* hit ){
   
   
   std::stringstream info;
   
   double x = hit->getPosition()[0];
   double y = hit->getPosition()[1];
   double z = hit->getPosition()[2];
   
   info << "(" << x << "," << y << "," << z << ")";
   
   return info.str();
   
}

std::string RecoTrack::getRecoTrackInfo() const{
   
   
   std::stringstream info;
   
   // the type of the track
   info << TRACK_TYPE_NAMES[ _type ] << " ";
   
   //the positions of the hits
   std::vector< TrackerHit* > hits = _track->getTrackerHits();
   std::sort( hits.begin(), hits.end(), KiTrackMarlin::compare_TrackerHit_z );
   for (unsigned i=0; i<hits.size(); i++ ) info << positionInfo( hits[i] );
   
   
   // the chi2 prob
   try{
      
      Fitter fitter( _track, _trkSystem );
      info << "Chi2Prob = " << fitter.getChi2Prob( lcio::TrackState::AtIP ) << "\n";
   }
   catch( FitterException e ){
      
      info << "Could not be fitted!!!\n";
      
   }
   
   return info.str();
   
   
}

