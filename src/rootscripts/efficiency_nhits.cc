#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"

#include <iostream>
#include <string>
#include <vector>




void efficiency_nhits(){
   
   
   
   /**********************************************************************************************/
   /*               Steering                                                                     */
   /**********************************************************************************************/
   
   //---------- Paths to load and save:
      
   const string MYPATH = "./";                                  // the current path
   std::vector< std::string > LOAD_FILE_NAMES;                    // the root files to be loaded
//    LOAD_FILE_NAMES.push_back( MYPATH + "Feedback.root" );
//    LOAD_FILE_NAMES.push_back( MYPATH + "FeedbackForwardHighPt.root" );
//    LOAD_FILE_NAMES.push_back( MYPATH + "FeedbackForwardLowPt.root" );
//    LOAD_FILE_NAMES.push_back( MYPATH + "FeedbackForwardBoth.root" );
   LOAD_FILE_NAMES.push_back( MYPATH + "FeedbackForward.root" );
   LOAD_FILE_NAMES.push_back( MYPATH + "FeedbackSilicon.root" );
   LOAD_FILE_NAMES.push_back( MYPATH + "FeedbackSubset.root" );
   
   std::vector< std::string > LOAD_FILE_MEANINGS;
//    LOAD_FILE_MEANINGS.push_back( "ForwardTrackingHighPt" );
//    LOAD_FILE_MEANINGS.push_back( "ForwardTrackingLowPt" );
//    LOAD_FILE_MEANINGS.push_back( "ForwardTrackingBoth" );
   LOAD_FILE_MEANINGS.push_back( "ForwardTracking" );
   LOAD_FILE_MEANINGS.push_back( "SiliconTracking" );
   LOAD_FILE_MEANINGS.push_back( "TrackSubsetProcessor" );
   
   const string TREENAME = "trueTracks";                                // name of the tree
   const string PICTURE_NAME = "Efficiency_nhits";
   const string PICTURE_ENDING = ".svg";
   const string PICTURE_SAVE_PATH = MYPATH + PICTURE_NAME + PICTURE_ENDING;     // where the image will be saved
   
   
   
   //---------- Optical settings:
    
   gROOT->SetStyle("Plain");    // a style using white instead of this horrible grey
   TCanvas* myCanvas = new TCanvas("myCanvas", "myCanvas", 0, 0, 600, 400);     //"new"-Command ist notwendig, damit die Canvas erhalten bleibt.

   TLegend* legend = new TLegend( 0.4, 0.15, 0.7, 0.35 );
   legend->SetFillColor( kWhite );
   
   //---------- Values for the histograms:
   int nBins = 20;
   double xMin = 0;
   double xMax = 30;
   double markerSize = 1.;
   
   
   TMultiGraph *mg = new TMultiGraph();
   mg->SetTitle( "Efficiency");
   
   
   
   /**********************************************************************************************/
   /*               The processing of the data                                                   */
   /**********************************************************************************************/
   
   
   for( unsigned i=0; i < LOAD_FILE_NAMES.size(); i++ ){
      
      
      std::string LOAD_FILE_NAME = LOAD_FILE_NAMES[i];
      std::string LOAD_FILE_MEANING = LOAD_FILE_MEANINGS[i];
      
      std::cout<< "loading " << LOAD_FILE_NAME << "\n";
      TFile* datafile = new TFile(LOAD_FILE_NAME.c_str());
      TTree* datatree = datafile->Get(TREENAME.c_str());
      
      
      TH1D *histAll = new TH1D("histAll","",nBins, xMin, xMax );
      TH1D *histFound = new TH1D("histFound","",nBins, xMin, xMax ); 
      
      
      //---------- linking the branch values to our local variables:
      
      int nHits;
      int nComplete;
      int nCompletePlus;
      int nIncomplete;
      int nIncompletePlus;
      
      
      datatree->SetBranchAddress ("nHits" , &nHits);
      datatree->SetBranchAddress ("nComplete" , &nComplete);
      datatree->SetBranchAddress ("nCompletePlus" , &nCompletePlus);
      datatree->SetBranchAddress ("nIncomplete" , &nIncomplete);
      datatree->SetBranchAddress ("nIncompletePlus" , &nIncompletePlus);
      
      
      
      int nEntries = datatree->GetEntries();       //numbers of entries in the tree
      std::cout<< "There are " << nEntries << " entries\n";
      
      //--------- Fill the histograms:
      
      for (int j = 0; j< nEntries; j++){
         
         datatree->GetEntry(j);
         
         histAll->Fill(nHits);
         
         
         int nFoundTracks = nComplete + nCompletePlus + nIncomplete + nIncompletePlus;
         if( nFoundTracks > 0 ) histFound->Fill( nHits );
         
         
      }
      
      
      
      //--------- Combine the two histograms to make an efficiency like plot
      
      TGraphAsymmErrors* histEfficiency = new TGraphAsymmErrors( histFound, histAll );
      int j = i+2;
      if( i==0 ) j=3;
      if( i==1 ) j=2;
      if( i==2 ) j=4;
      histEfficiency->SetMarkerColor( j );
      histEfficiency->SetMarkerStyle( i+20 );
      histEfficiency->SetMarkerSize( markerSize );
      histEfficiency->SetLineColor( j );
      
      
      legend->AddEntry( histEfficiency, LOAD_FILE_MEANING.c_str() );
      
      mg->Add( histEfficiency );
      
   }

   mg->Draw("AP");
   mg->GetYaxis()->SetRangeUser(0.,1.);
   mg->GetXaxis()->SetTitle( "number of hits" );

   legend->Draw("same");

   myCanvas->Update();
   myCanvas->SaveAs( PICTURE_SAVE_PATH .c_str());    //Save the data to an image file
   
   
   
   
}