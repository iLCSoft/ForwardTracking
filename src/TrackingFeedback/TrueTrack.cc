#include "TrueTrack.h"

#include <sstream>
#include <cmath>
#include "Fitter.h"
#include "UTIL/ILDConf.h"
#include "FTrackTools.h"
#include <algorithm>

std::string TrueTrack::getMCPInfo(){
   
   std::stringstream mcpInfo;   
   
   
      
   double px = _mcp->getMomentum()[0];
   double py = _mcp->getMomentum()[1];
   double pz = _mcp->getMomentum()[2];
   double pt = sqrt( px*px + py*py );    
   double p= sqrt( px*px + py*py + pz*pz );
   
   mcpInfo.precision (3);
   mcpInfo.setf(std::ios::fixed);
   
   mcpInfo << "px= " << px << ", py= " <<  py << ", pz= " <<  pz << ", pt= " << pt << ", p= " << p << "\n";
   mcpInfo << "Vertex= (" << _mcp->getVertex()[0] << ", " <<  _mcp->getVertex()[1] << ", " <<  _mcp->getVertex()[2] << ")\tPDG= " << _mcp->getPDG() << "\n";
  
   //          mcpInfo << "/gun/direction " << px/p << " " <<  py/p << " " <<  pz/p << std::endl;
   
   
   
   
   
   
   return mcpInfo.str();
   
   
}

std::string TrueTrack::getTrueTrackInfo(){
   
   
   std::stringstream trackInfo;   
   
      
   
   // The Fit Information 
   Fitter fitter( _trueTrack );
   trackInfo << "Chi2Prob = " << fitter.getChi2Prob() 
               << ", Chi2 = " << fitter.getChi2() 
               << ", Ndf = " << fitter.getNdf() << "\n";
               
   // Information about the hits:
   std::vector< TrackerHit* > hits= _trueTrack->getTrackerHits();
   
   for( unsigned i=0; i < hits.size(); i++ ){
      
      TrackerHit* hit = hits[i];
      
      trackInfo << "\t" << positionInfo(hit) <<  "\t" << cellIDInfo(hit) << "\n";
      
   }
  
   
   
   
   
   
   return trackInfo.str();
   
}

std::string TrueTrack::cellIDInfo( TrackerHit* hit ){
   
   
   std::stringstream info;
   
   UTIL::BitField64  cellID( ILDCellID0::encoder_string );
   cellID.setValue( hit->getCellID0() );
   int side   = cellID[ ILDCellID0::side ];
   int layer = cellID[ ILDCellID0::layer ];
   int module = cellID[ ILDCellID0::module ];
   int sensor = cellID[ ILDCellID0::sensor ];

   info << "side " << side << ", layer " << layer << ", module " << module << ", sensor " << sensor;
   
   return info.str();
   
}

std::string TrueTrack::positionInfo( TrackerHit* hit ){
   
   
   std::stringstream info;
   
   double x = hit->getPosition()[0];
   double y = hit->getPosition()[1];
   double z = hit->getPosition()[2];
   
   info << "(" << x << "," << y << "," << z << ")";
   
   return info.str();
   
}


std::string TrueTrack::getRelatedTracksInfo(){
   
   
   std::stringstream info;
   
   std::map<Track*,TrackType>::iterator it;
   
   info << "Related Tracks:\n";
   
   // for all related tracks
   for( it= map_track_type.begin(); it != map_track_type.end(); it++ ){
      
      Track* track = it->first;
      TrackType type = it->second;
      
      // the type of the track
      info << TRACK_TYPE_NAMES[ type ] << " ";
      
      //the positions of the hits
      std::vector< TrackerHit* > hits = track->getTrackerHits();
      std::sort( hits.begin(), hits.end(), FTrack::compare_TrackerHit_z );
      for (unsigned i=0; i<hits.size(); i++ ) info << positionInfo( hits[i] );
      
      
      // the chi2 prob
      Fitter fitter( track );
      info << "Chi2Prob = " << fitter.getChi2Prob() << "\n";
      
      
   }
   
   return info.str();
   
   
}

std::string TrueTrack::getFoundInfo(){
   
 
   std::string info = "Track is ";
   
   if( isLost ) info += "LOST";
   else         info += "FOUND";
   
   if( completeVersionExists ) info += " and there is a COMPLETE version.";
   
   info += "\n";
   
   return info;
   
   
   
}


   