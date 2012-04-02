#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Tools/Timer.h"

/**
 * This small executable is there to automically run Marlin for different values of the 
 * criteria. It is half hardcoded so beware.
 */

int main(int argc,char *argv[]){

   
   const std::string STEERING_FILE = "/scratch/ilcsoft/Steers/automaton_steer.xml";
   const std::string FEEDBACK_FILE = "/scratch/ilcsoft/Steers/FeedbackSum.csv";
   
   const std::string QUANTILE_ANALYSER_PATH = "/scratch/ilcsoft/trunk/ForwardTracking/trunk/build/bin/QuantileAnalyser";
   const std::string QUANTILE_ANALYSER_TARGET = "/scratch/ilcsoft/Steers01/savedData/TrueTracksCritAnalysis_1000evts.root";
   const std::string QUANTILE_ANALYSER_OUTPUT = "/scratch/ilcsoft/Steers01/quantile_analyser_output";
   
   
   for( float quantSize = 0.9; quantSize <= 1.0 ; quantSize += 0.1 ){
      
      
      
      /**********************************************************************************************/
      /*               Get the ranges of the quantiles                                              */
      /**********************************************************************************************/

      // run the QuantileAnalyser
      

      std::stringstream command;
      command << QUANTILE_ANALYSER_PATH << " " << quantSize<< " " << QUANTILE_ANALYSER_TARGET << " " << QUANTILE_ANALYSER_OUTPUT;
      int returnValue = system( command.str().c_str() );
      
      if( returnValue != 0 ){
         
         std::cout << "\n\n quantile analyser did not return 0. Error!!!\n\n";
         return 1;
         
      }
      
      // get the quantile info
      std::string parameters;
      
      std::ifstream quantFile ( QUANTILE_ANALYSER_OUTPUT.c_str() );
      if (quantFile.is_open())
      {
         while ( quantFile.good() )
         {
            std::string line;
            getline (quantFile,line);
            parameters += line;
         }
         quantFile.close();
      }
      
   
      
      /**********************************************************************************************/
      /*                Run Marlin                                                                  */
      /**********************************************************************************************/
      command.str("");
      command << "Marlin " << STEERING_FILE << " " << parameters;
      returnValue = system( command.str().c_str() );

      if( returnValue != 0 ){
         
         std::cout << "\n\n Marlin did not return 0. Error!!!\n\n";
         return 1;
         
      }
      

      std::ofstream myfile;
      myfile.open (FEEDBACK_FILE.c_str() , std::ios::app);
   
      
      myfile << "quantile size\t" << quantSize << "\t\t";
      
      
      myfile << "elapsed time\t" << KiTrackMarlin::Timer::lap();
      
      
      myfile.close();
   
   }
   
   
   return 0;

}

