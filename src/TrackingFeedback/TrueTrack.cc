#include "TrueTrack.h"

#include <sstream>
#include <cmath>
#include <algorithm>


#include "Tools/Fitter.h"
#include "Tools/KiTrackMarlinTools.h"


unsigned TrueTrack::getNumberOfTracksWithType( TrackType type ) const{
   
   unsigned n=0;
   
   for( unsigned i=0; i < _recoTracks.size(); i++ ){
      
      if( _recoTracks[i]->getType() == type ) n++;
      
   }
   
   return n;
   
   
}


bool TrueTrack::isLost() const{
   
   if( _recoTracks.size() == 0 ) return true;
  
   return false;
   
}
   
   
bool TrueTrack::isFoundCompletely() const{
   
   
   return getNumberOfTracksWithType( COMPLETE ) + getNumberOfTracksWithType( COMPLETE_PLUS );
   
}


bool TrueTrack::completeVersionExists() const{
   
   return getNumberOfTracksWithType( COMPLETE );
   
}



std::string TrueTrack::getMCPInfo() const{
   
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

std::string TrueTrack::getTrueTrackInfo() const{
   
   
   std::stringstream trackInfo;   
   
   
   // The Fit Information 
   
   
   try{
      
      Fitter fitter( _trueTrack, _trkSystem );
      trackInfo << "Chi2Prob = " << fitter.getChi2Prob( lcio::TrackState::AtIP ) 
      << ", Chi2 = " << fitter.getChi2( lcio::TrackState::AtIP ) 
      << ", Ndf = " << fitter.getNdf( lcio::TrackState::AtIP ) << "\n";
      
   }
   catch( FitterException e ){
      
      trackInfo << "Could not be fitted!!!\n";
      
   }
   
   
   // Information about the hits:
   std::vector< TrackerHit* > hits= _trueTrack->getTrackerHits();
   
   std::sort( hits.begin(), hits.end(), KiTrackMarlin::compare_TrackerHit_z );
   
   for( unsigned i=0; i < hits.size(); i++ ){
      
      TrackerHit* hit = hits[i];
      
      trackInfo << "\t" << hit << "\t"<< RecoTrack::positionInfo(hit) <<  "\t" << RecoTrack::cellIDInfo(hit) << "\n";
      
   }
  
   
   
   return trackInfo.str();
   
}



std::string TrueTrack::getRelatedTracksInfo() const{
   
   
   std::stringstream info;
   
   
   info << "Related Tracks:\n";
   
   // for all related tracks
   for( unsigned i=0; i < _recoTracks.size(); i++ ){
      
      
      info << _recoTracks[i]->getRecoTrackInfo();
      
      
   }
   
   return info.str();
   
   
}

std::string TrueTrack::getFoundInfo() const{
   
 
   std::string info = "Track is ";
   
   if( isLost() ) info += "LOST";
   else           info += "FOUND";
   
   if( completeVersionExists() ) info += " and there is a COMPLETE version.";
   
   info += "\n";
   
   return info;
   
   
   
}

