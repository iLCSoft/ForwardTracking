#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

#include "TFile.h"
#include "TTree.h"

#include "Criteria.h"


using namespace FTrack;


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



int main(int argc,char *argv[]){
   
   std::string ROOT_FILE_PATH = "/scratch/ilcsoft/Steers/TrueTracksCritAnalysis.root";
   if( argc >= 3 ) ROOT_FILE_PATH = argv[2];

   float quantile = 1.;
   
   if( argc >= 2 ) quantile = atof( argv[1] );
   
   
   
   /**********************************************************************************************/
   /*                Open ROOT file                                                              */
   /**********************************************************************************************/
   
   TFile* rootFile = new TFile( ROOT_FILE_PATH.c_str() , "READ"); 
    
   
   Criteria::init();
   std::vector< std::string > critTypes = Criteria::getTypes();
   
   
   
   
   for( unsigned i = 0; i < critTypes.size(); i++ ){ // once for every type of criteria ( 1 type = 1 tree in ROOT file )
      
      
      std::map < std::string , std::vector <float> > map_name_value;
      
      
      /**********************************************************************************************/
      /*                Read out the tree                                                           */
      /**********************************************************************************************/
      
      std::string critType = critTypes[i];
      std::string treeName = critType;
      std::cout << "\n" << critType;
   
      TTree* tree = (TTree*) rootFile->Get( treeName.c_str() );
      unsigned nTreeEntries = tree->GetEntries();
      
      std::vector< std::string > crits = Criteria::getCriteria( critType );
      
      
      for (unsigned j = 0; j< nTreeEntries; j++){
      
         for( unsigned k=0; k < crits.size() ; k++){
            
            
            std::string critName = crits[k];
            
            std::string branchName = critName + "_" + critName;
            
            map_name_value[ critName ].push_back( 0. );
            
            tree -> SetBranchAddress ( branchName.c_str() , &(map_name_value[ critName ].back()) );
            
             
         }
       
         tree->GetEntry(j);
       
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
         
         calcMinMaxOfQuantile( values , min , max , quantile , 0.5 , 0.5 );
         
         min -= 0.01*fabs(min); // So that the actual minimum is not the boarder, but inside by a little bit
         max += 0.01*fabs(max);
         
//          std::cout << "\n" << critName << ": min = " << min << ", max = " << max;
         std::cout << "\n\t" << "Crit" << critType[0] << "_" << critName 
                   << "( " << min << " , " << max << " )";
         
      }
      
      /**********************************************************************************************/
      /*                Save the information                                                        */
      /**********************************************************************************************/

   
   }
   
   
   delete rootFile;
   
   
   
   
   return 0;
   
}