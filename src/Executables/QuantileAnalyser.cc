#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <cmath>

#include "TFile.h"
#include "TTree.h"

#include "Criteria/Criteria.h"


using namespace KiTrack;



/** Calculates the minimum and maximum value, so that all values between them are inside a quantile.
 * 
 * @param values a vector of the values for which the quantile is checked
 * 
 * @param min passed by reference: here the minimum will be stored
 * 
 * @param max passed by reference: here the maximum will be stored
 * 
 * @param quantile a value between 0 and 1 that says how many percent of the data shall be inside the quantile.
 * 
 * 
 * @param partLeft a value between 0 and 1 saying how many percent of the value that lies outside the quantile shall be 
 * below the minimum.
 * 
 *  @param partRight a value between 0 and 1 saying how many percent of the value that lies outside the quantile shall be 
 * above the minimum.
 * If partLeft and partRight don't add up to 1 they will be normed, so that they are. So 3 and 1 will give 0.25 and 0.75.
 * 
 */
void calcMinMaxOfQuantile( std::vector< float > values, float &min, float &max, float quantile , float partLeft = 0.5 , float partRight = 0.5 ){
   
   if ( values.empty() ) return;
   
   if (quantile < 0) quantile = 0.;
   if (quantile > 1.) quantile = 1.;
   
   // Norm partLeft and partRight
   partLeft = partLeft/ (partLeft + partRight);
   partRight = partRight/ (partLeft + partRight);
   
   sort( values.begin() , values.end() ); // sort values from low to big
   
   unsigned nOutsideQuantile =  unsigned( values.size() * (1. - quantile) );
   
   unsigned nOutsideLeft = unsigned ( round ( nOutsideQuantile * partLeft ) );
   unsigned nOutsideRight = unsigned ( round ( nOutsideQuantile * partRight ) );
   
   min = values[nOutsideLeft];
   max = values[values.size() - nOutsideRight - 1];
   
   return;
   
}

/**
 * @param argv[1] quantile size
 * 
 * @param argv[2] root file path
 * 
 * @param argv[3] output path
 * 
 */
int main(int argc,char *argv[]){
   
   
   float quantile = 1.;
   
   if( argc >= 2 ) quantile = atof( argv[1] );
   
   
   std::string ROOT_FILE_PATH = "/home/robin/Desktop/TrueTracksCritAnalysis.root";
   if( argc >= 3 ) ROOT_FILE_PATH = argv[2];

   std::string OUTPUT_PATH = "quantile_analyser_output";
   if( argc >= 4 ) OUTPUT_PATH = argv[3];
   
   
   ofstream myfile;
   myfile.open (OUTPUT_PATH.c_str() );
   
   
   const float chi2ProbMin = 0.00;

   
   
   
   /**********************************************************************************************/
   /*                Open ROOT file                                                              */
   /**********************************************************************************************/
   
   TFile* rootFile = new TFile( ROOT_FILE_PATH.c_str() , "READ"); 

   

   std::set< std::string > critTypes = Criteria::getTypes();
   
   std::set< std::string >::iterator iType;
   
   std::stringstream steerInfo("\n\n"); //for getting something that can be used in the marlin steer file
   std::stringstream steerInfob; // a second part
   
   for( iType = critTypes.begin(); iType != critTypes.end(); iType++ ){ // once for every type of criteria ( 1 type = 1 tree in ROOT file )
      
         
      std::map < std::string , std::vector <float> > map_name_value;
      
      
      /**********************************************************************************************/
      /*                Read out the tree                                                           */
      /**********************************************************************************************/
      
      std::string critType = *iType;
      std::string treeName = critType;
      std::cout << "\n" << critType;
   
      TTree* tree = (TTree*) rootFile->Get( treeName.c_str() );
      unsigned nTreeEntries = tree->GetEntries();
      
      std::set< std::string > crits = Criteria::getCriteriaNames( critType );
      std::set< std::string >::iterator iCrit;
      
      
      
      
      
      for (unsigned j = 0; j< nTreeEntries; j++){
         
         
         float chi2Prob = 0.;
         tree -> SetBranchAddress ( "chi2Prob" , &chi2Prob );
         tree->GetEntry(j);
         
         for( iCrit = crits.begin(); iCrit != crits.end() ; iCrit++){
            
            
            std::string critName = *iCrit;
            
            std::string branchName = critName;
            
            if( chi2Prob > chi2ProbMin ) map_name_value[ critName ].push_back( 0. ); // only if chi2prob is good
            
            TBranch* branch = tree->GetBranch( branchName.c_str() );
            if (branch) {
               
               tree->SetBranchAddress ( branchName.c_str() , &(map_name_value[ critName ].back()) );
               
            }
            else continue; // if there is no branch with that name skip it
           
            
            
         }
       
       if( chi2Prob > chi2ProbMin )tree->GetEntry(j); // only if chi2prob is good
       
      }
   
      
   
      /**********************************************************************************************/
      /*                Analyse the Quantiles                                                       */
      /**********************************************************************************************/
      
      // Now we have all our data from the tree stored in the vectors in the map. 
      
      std::map < std::string , std::vector <float> >::iterator it;
      
      for( it= map_name_value.begin(); it != map_name_value.end(); it++ ){
         
         
         std::string critName = it->first;
         std::vector < float > values = it->second;
      
         float min = 0.;
         float max = 0.;
         
         float left = 0.5;
         float right = 0.5;
         Criteria::getLeftRight( critName, left, right );
         
         calcMinMaxOfQuantile( values , min , max , quantile , left , right );
         
//          min -= 0.01*fabs(min); // So that the actual minimum is not the boarder, but inside by a little bit
//          max += 0.01*fabs(max);
         
         std::cout << "\n" << critName << ": min = " << min << ", max = " << max;
         steerInfo << "\n<parameter name=\"" << critName << "_min\" type=\"float\">" << min << "</parameter>";
         steerInfo << "\n<parameter name=\"" << critName << "_max\" type=\"float\">" << max << "</parameter>";
         steerInfob << critName << "\n";
         
         
         myfile << "--MyForwardTracking." << critName << "_min=" << min << "   ";
         myfile << "--MyForwardTracking." << critName << "_max=" << max << "   ";
         
         
         
      }
      
      std::cout << "\n\n" ;
      steerInfo << "\n\n";
      
      
      
   }
   
   steerInfo << "<parameter name=\"Criteria\" type=\"StringVec\">";
   steerInfo << steerInfob.str();
   steerInfo << "</parameter>\n\n";
   
   std::cout << steerInfo.str();
   
   delete rootFile;
   

   myfile.close();
   

   
   
   
   return 0;
   
}

