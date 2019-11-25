#include "SectorSystemEndcap.h"

#include <sstream>
#include <cmath>

using namespace KiTrackMarlin;


SectorSystemEndcap::SectorSystemEndcap( unsigned nLayers, unsigned nDivisionsInPhi, unsigned nDivisionsInTheta ){   

  _nLayers = nLayers;
  _nDivisionsInPhi = nDivisionsInPhi ;
  _nDivisionsInTheta = nDivisionsInTheta ;
  _sectorMax = _nLayers + _nLayers*_nDivisionsInPhi + _nLayers*_nDivisionsInPhi*_nDivisionsInTheta ;
   
}

unsigned SectorSystemEndcap::getNLayers() const {

  return _nLayers ;

}

unsigned SectorSystemEndcap::getPhiSectors() const {

  return _nDivisionsInPhi ;

}


unsigned SectorSystemEndcap::getThetaSectors() const {

  return _nDivisionsInTheta ;

} 
  

unsigned SectorSystemEndcap::getLayer( int sector ) const {
  
  //std::cout << " SectorSystemEndcap::getLayer  total no of layers = " << _nLayers << " n divisions in phi = " << _nDivisionsInPhi << " sector = " << sector << std::endl ;
  
  int theta = sector/(_nLayers*_nDivisionsInPhi) ;
  
  int phi = ((sector - (theta*_nLayers*_nDivisionsInPhi)) / _nLayers) ;
  
  int Layer = sector - (theta*_nLayers*_nDivisionsInPhi) - (phi*_nLayers) ; 

  //std::cout << " SectorSystemEndcap::getLayer " << Layer << " total no of layers = " << _nLayers << " iPhi = " << phi << " iTheta = " << theta << " n divisions in phi = " << _nDivisionsInPhi << " sector = " << sector << std::endl ;
  
  return Layer ;
  
}


unsigned SectorSystemEndcap::getPhi( int sector) const {

  int theta = sector/(_nLayers*_nDivisionsInPhi) ;

  int Phi = ((sector - (theta*_nLayers*_nDivisionsInPhi)) / _nLayers) ;

  //std::cout << " SectorSystemEndcap::getPhi " << Phi << std::endl ;
   
  return Phi ;
   
}


unsigned SectorSystemEndcap::getTheta( int sector ) const {

   int Theta = sector/(_nLayers*_nDivisionsInPhi) ;

   //std::cout << " SectorSystemEndcap::getTheta " << Theta << std::endl ;

   return Theta ;
      
}


int SectorSystemEndcap::getSector( int layer , int phi , int theta ) const {
  
  //std::cout << "getting sector : layer " << layer << " phi " << phi << " theta " << theta << std::endl ;

  if ( layer >= int(_nLayers) ){
    
    std::stringstream s; 
    s << "Layer " << layer << " is too big, the outermost layer is layer " << _nLayers - 1 ;
    throw OutOfRange( s.str() );
    
  }

  
  if ( phi >= int(_nDivisionsInPhi) ){
    
    std::stringstream s; 
    s << "Phi " << phi << " is too big, the highest phi division is " << _nDivisionsInPhi ;
    throw OutOfRange( s.str() );
    
  }


  
  if ( theta >= int(_nDivisionsInTheta) ){
    
    std::stringstream s;
    s << "Theta " << theta << " is too big, the highest theta division is " << _nDivisionsInTheta ;

    throw OutOfRange( s.str() );
    //std::cout << " ####3 calling getSector function $$$$$$$$$$" << std::endl ;    
  }   

  int sector = layer + _nLayers*phi + _nLayers*_nDivisionsInPhi*theta ;
  //std::cout << " did you call me? I am the Theta version, give you the sector " << sector << std::endl ;
    
  return sector ;  

}


int SectorSystemEndcap::getSector( int layer , double phi , double cosTheta ) const {
  

  double _dPhi = (2*M_PI)/_nDivisionsInPhi;
  double _dTheta = 2.0/_nDivisionsInTheta;
  int iPhi = int(phi / _dPhi);
  int iTheta = int ((cosTheta + double(1.0))/_dTheta);

  //std::cout << "getting sector : layer " << layer << " phi " << iPhi << " theta " << iTheta << std::endl ;

  if ( layer >= int(_nLayers) ){
    
    std::stringstream s; 
    s << "Layer " << layer << " is too big, the outermost layer is layer " << _nLayers - 1 ;
    throw OutOfRange( s.str() );
    
  }

  
  if ( iPhi >= int(_nDivisionsInPhi) ){
    
    std::stringstream s; 
    s << "Phi " << iPhi << " is too big, the highest phi division is " << _nDivisionsInPhi ;
    throw OutOfRange( s.str() );
    
  }


  
  if ( iTheta >= int(_nDivisionsInTheta) ){
    
    std::stringstream s;
    s << "Theta " << iTheta << " is too big, the highest theta division is " << _nDivisionsInTheta ;

    throw OutOfRange( s.str() );
    //std::cout << " ####3 calling getSector function $$$$$$$$$$" << std::endl ;    
  }   

  int sector = layer + _nLayers*iPhi + _nLayers*_nDivisionsInPhi*iTheta ;
  //std::cout << " did you call me? I am the cosTheta version, give you the sector " << sector << std::endl ;
  
  return sector ;  

}



void SectorSystemEndcap::checkSectorIsInRange( int sector ) const {


   if ( sector > _sectorMax ){
      
      std::stringstream s;
      s << "SectorSystemVXS:\n Sector " 
        << sector << " is too big, the highest possible number for a sector in this configuration of VXD - SIT is"
        << _sectorMax 
        << ".\nThe configuration is: nLayers = " << _nLayers
        << ", n divisions in phi = " << _nDivisionsInPhi
        << ", n divisions in theta = " << _nDivisionsInTheta
	<< _nLayers + _nLayers*_nDivisionsInPhi + _nLayers*_nDivisionsInPhi*_nDivisionsInTheta ;
      throw OutOfRange( s.str() );
      
   }  

}


std::string SectorSystemEndcap::getInfoOnSector( int sector ) const{
   
   
   std::stringstream s;
   s << " (layer" << getLayer(sector )  
     << ",theta" << getTheta(sector )
     << ",phi" << getPhi(sector ) 
     << ")";
   
   
   return s.str();   
   
   
}


