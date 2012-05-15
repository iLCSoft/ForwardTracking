#include "TROOT.h"
#include "TFile.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

/* This small rootscript is there to make a diagram from a csv file in a certain format
 */
void make_diagram_from_csv(){

   
   
   //---------- Optical settings:
   
   gROOT->SetStyle("Plain");    // a style using white instead of this horrible grey
   TCanvas* myCanvas = new TCanvas("myCanvas", "myCanvas", 0, 0, 600, 400);     //"new"-Command ist notwendig, damit die Canvas erhalten bleibt.
   TLegend* legend = new TLegend( 0.15, 0.65, 0.4, 0.85 );
   legend->SetFillColor( kWhite );
   
   
   const string MYPATH = "./"; 
   const std::string CSV_FILE = MYPATH + "time_background.csv";
   const string PICTURE_NAME = "time";
   const string PICTURE_ENDING = ".svg";
   const string PICTURE_SAVE_PATH = MYPATH + PICTURE_NAME + PICTURE_ENDING;     // where the image will be saved
   const string TITLE = "Time per event [s]";
//    const string TITLE = "Ghostrate";
   
   
   const std::string xName = "Background";
   
   
   std::map< std::string, std::vector<double> > map_names_values;
   
   std::vector< std::string > yNames;
   yNames.push_back( "Time SiliconTracking" );
   yNames.push_back( "Time ForwardTracking" );
   
   
   ifstream csvFile;
   csvFile.open( CSV_FILE.c_str() );
   
   
   
   // read in the names of the values
   
   std::string line;
   std::getline(csvFile, line); // read in the first line with the names of the file
   
   std::vector< std::string > names;
   std::stringstream linestream(line);
   std::string name;
   
   while( std::getline( linestream, name, ';' ) ){
      
      names.push_back( name );
      std::cout<< name << "\n";
      
   }
   
   // now read in the values
   
   std::vector< float > values;
   
   
   
   while(std::getline(csvFile, line))
   {
      
      std::stringstream stream( line );
      
      double value;
      unsigned i=0;
      
      while( stream >> value ){
         
         std::cout << value << "\t";
         
         map_names_values[ names[i] ].push_back( value );
         
         if (stream.peek() == ';') stream.ignore();
         
         i++;
      }
      
      std::cout << "\n";
      
      
      
      
   }
   
   
   csvFile.close();
  
   
   
   // Now the map is filled with the names and values, time to make the histograms
   std::vector< double > x = map_names_values[ xName ];
   std::vector< double > y;

   // The following is needed because TGraph want arrays
   const int n= 50;
   double xArray[ n ];
   double yArray[ n ];
   
   TMultiGraph *mg = new TMultiGraph();
   mg->SetTitle( TITLE.c_str() );
   
   
   for( unsigned i=0; i< yNames.size(); i++ ){
      
      std::cout << i << "\n";
      
      y = map_names_values[ yNames[i] ];
      
      if (x.size() != n ) std::cout << "ERROR, size of " << xName << "is not equal to n=" << n << "but is " << x.size() << "\n";
      if (y.size() != n ) std::cout << "ERROR, size of " << yNames[i] << "is not equal to n=" << n << "but is " << y.size() << "\n";
      
      for( int j=0; j<n; j++ ){
         
         std::cout << j << "\n";
         xArray[j] = x[j];
         yArray[j] = y[j];
         
      }
      
      
      TGraph* graph = new TGraph( n, xArray, yArray );
      graph->SetMarkerColor( i+2 );
      graph->SetMarkerStyle( i+20 );
      graph->SetMarkerSize( 0.5 );
      graph->SetLineColor( i+2 );
      mg->Add( graph );
      
      legend->AddEntry( graph, yNames[i].c_str() , "LP" );
      
//       if( i == 0 )graph->Draw("APL"); // for info on why "AP" see http://root.cern.ch/root/html/TGraphPainter.html
//       else        graph->Draw("samePL");
      
   }
   
   mg->Draw("APL");
//    mg->GetYaxis()->SetRangeUser(0.,1.);
   mg->GetXaxis()->SetTitle( xName.c_str() );
   
   legend->Draw("same");
   
   myCanvas->Update();
   myCanvas->SaveAs( PICTURE_SAVE_PATH .c_str());    //Save the data to an image file
   

}