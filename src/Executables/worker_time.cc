#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Tools/Timer.h"

/**
 * This small executable is there to automically run Marlin for different values of the 
 * background, so afterwards time and efficiency etc. can be compared.
 * It is mostly hardcoded, so beware!
 */

int main(int argc,char *argv[]){

   
   const std::string STEERING_FILE = "/scratch/ilcsoft/Steers01/automaton_steer.xml";
   const std::string FEEDBACK_FILE1 = "/scratch/ilcsoft/Steers01/Feedback/TrackingFeedbackSum.csv";
   const std::string FEEDBACK_FILE2 = "/scratch/ilcsoft/Steers01/Feedback/TrackingFeedbackSum_Sil.csv";
   
   const float densityRegulatorMin = 0.;
   const float densityRegulatorMax = 2.;
   const float densityRegulatorSteps = 2.;
   
   for( float regulator = densityRegulatorMin; regulator <= densityRegulatorMax; regulator += densityRegulatorSteps ){
      
      
       
      // set the parameter for 
      std::stringstream parameters;
      parameters << " --MyFTDBackgroundProcessor01.DensityRegulator=" << regulator ;
      
      
   
      
      /**********************************************************************************************/
      /*                Run Marlin                                                                  */
      /**********************************************************************************************/
      std::string command = "Marlin " + STEERING_FILE + " " + parameters.str();
      int returnValue = system( command.c_str() );
      
      if( returnValue != 0 ){
         
         std::cout << "\n\n Marlin did not return 0. Error!!!\n\n";
         return 1;
         
      }
      
      
      std::ofstream myfile;
      myfile.open (FEEDBACK_FILE1.c_str() , std::ios::app);
      myfile << "DensityRegulator\t" << regulator << "\t\t";
      myfile.close();
      
      myfile.open (FEEDBACK_FILE2.c_str() , std::ios::app);
      myfile << "DensityRegulator\t" << regulator << "\t\t";
      myfile.close();
      
   }
   
   
   return 0;

}

