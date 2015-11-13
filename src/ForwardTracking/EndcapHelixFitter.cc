#include "EndcapHelixFitter.h"

#include <sstream>
#include <algorithm>
#include <cmath>

#include "EVENT/TrackerHitPlane.h"
#include "UTIL/ILDConf.h"
#include "marlin/VerbosityLevels.h"
#include "MarlinTrk/HelixFit.h"

#include "Tools/KiTrackMarlinTools.h"


EndcapHelixFitter::EndcapHelixFitter( std::vector< TrackerHit* > trackerHits )throw( EndcapHelixFitterException ){
   
   _trackerHits = trackerHits;
   
   fit();
   
}

EndcapHelixFitter::EndcapHelixFitter( Track* track )throw( EndcapHelixFitterException ){
   
   _trackerHits = track->getTrackerHits();
   
   fit();

}

void EndcapHelixFitter::fit()throw( EndcapHelixFitterException ){
   

   std::sort( _trackerHits.begin(), _trackerHits.end(), KiTrackMarlin::compare_TrackerHit_R );
   
   int nHits = _trackerHits.size();
   int iopt = 2;
   float chi2RPhi;
   float chi2Z;

   // DEBUG
   //std::cout << " no of hits fitted " << nHits << std::endl ; 
   
   if( nHits < 3 ){
      
      std::stringstream s;
      s << "EndcapHelixFitter::fit(): Cannot fit less with less than 3 hits. Number of hits =  " << nHits << "\n";
      
      throw EndcapHelixFitterException( s.str() );
      
   }
   
   double* xh  = new double[nHits];
   double* yh  = new double[nHits];
   float*  zh  = new float[nHits];
   double* wrh = new double[nHits];
   float*  wzh = new float[nHits];
   float*  rh  = new float[nHits];
   float*  ph  = new float[nHits];
   
   float par[5];
   float epar[15];
   
   for( int i=0; i<nHits; i++ ){
      
      
      TrackerHit* hit = _trackerHits[i];
      
      xh[i] = hit->getPosition()[0];
      yh[i] = hit->getPosition()[1];
      zh[i] = float(hit->getPosition()[2]);
      
      //wrh[i] = double(1.0/(hit->getResolutionRPhi()*hit->getResolutionRPhi()));
      //wzh[i] = 1.0/(hit->getResolutionZ()*hit->getResolutionZ());
      //wrh[i] = double(1.0/(sqrt((hit->getCovMatrix()[0]*hit->getCovMatrix()[0]) + (hit->getCovMatrix()[2]*hit->getCovMatrix()[2]))));
      //wzh[i] = 1.0/(hit->getCovMatrix()[5]);

      rh[i] = float(sqrt(xh[i]*xh[i]+yh[i]*yh[i]));
      ph[i] = atan2(yh[i],xh[i]);
      if (ph[i] < 0.) 
         ph[i] = 2.*M_PI + ph[i]; 
      
      // Just to debug the resolutions
      //std::cout << " hit's radius " << rh[i] << " R-phi uncertainty " << wrh[i] << " Z uncertainty " << wzh[i] << " Cov[0] " << hit->getCovMatrix()[0] << " Cov[2] " << hit->getCovMatrix()[2] << " Cov[5] " << hit->getCovMatrix()[5] << std::endl ;
      
      if( BitSet32( hit->getType() )[ UTIL::ILDTrkHitTypeBit::COMPOSITE_SPACEPOINT ] ){
         
         
         float sigX = hit->getCovMatrix()[0];
         float sigY = hit->getCovMatrix()[2];
         wrh[i] = 1/sqrt( sigX*sigX + sigY*sigY );
	 wzh[i] = 1.0/(hit->getCovMatrix()[5]);
         
	 streamlog_out(DEBUG4) << " SPACEPOINT:: hit's radius " << rh[i] << " R-phi uncertainty " << wrh[i] << " Z uncertainty " << wzh[i] << " res RPhi " << sqrt( sigX*sigX + sigY*sigY ) << " res Z " << (hit->getCovMatrix()[5]) << std::endl ;         
         
      }
      else {
         
         TrackerHitPlane* hitPlane = dynamic_cast<TrackerHitPlane*>( hit );
         wrh[i] = double(1.0/( hitPlane->getdU()*hitPlane->getdU() + hitPlane->getdV()*hitPlane->getdV() ) );
	 wzh[i] = wrh[i]; // Provisionary, for the pixel VXD - SIT
 
	 streamlog_out(DEBUG4) << " TRACKERHITPLANE:: hit's radius " << rh[i] << " R-phi uncertainty " << wrh[i] << " Z uncertainty " << wzh[i] << " dU " << hitPlane->getdU() << " dV " << hitPlane->getdV()  << " 1/du^2 "<< (1/(hitPlane->getdU()*hitPlane->getdU())) <<  " 1/dv^2 "<< (1/(hitPlane->getdV()*hitPlane->getdV())) << std::endl ;
        
      }
      
     
      
   }
   
   
   
   
   MarlinTrk::HelixFit helixFitter;
   
   helixFitter.fastHelixFit(nHits, xh, yh, rh, ph, wrh, zh, wzh,iopt, par, epar, chi2RPhi, chi2Z);
   par[3] = par[3]*par[0]/fabs(par[0]);
   
   _omega = par[0];
   _tanLambda = par[1];
   _phi0 = par[2];
   _d0 = par[3];
   _z0 = par[4];
   
   float chi2 = chi2RPhi+chi2Z;
   int Ndf = 2*nHits-5;
   
   
   
   
   delete[] xh;
   delete[] yh;
   delete[] zh;
   delete[] wrh;
   delete[] wzh;
   delete[] rh;
   delete[] ph;
   xh  = NULL; 
   yh  = NULL; 
   zh  = NULL; 
   wrh = NULL; 
   wzh = NULL; 
   rh = NULL; 
   ph = NULL; 
   
 
   streamlog_out(DEBUG4) << "chi2 rphi = " << chi2RPhi << ", chi2 Z = " << chi2Z << ", Ndf = " << Ndf << "\n";
   
   _chi2 = chi2;
   _Ndf = Ndf;
   
   return;
   
   
   
}




