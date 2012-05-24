#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"

#include <iostream>
#include <string>
#include <vector>

enum TrackType { COMPLETE , COMPLETE_PLUS , INCOMPLETE , INCOMPLETE_PLUS , GHOST , LOST };

void binLogX(TH1*h, std::string Saxis){
   
   
   TAxis *axis = 0;
   if(Saxis == "X") {
      axis = h->GetXaxis();
   } else if(Saxis == "Y") {
      axis = h->GetYaxis();
   } else {
      return;
   }
   int bins = axis->GetNbins();

   Axis_t from = axis->GetXmin();
   Axis_t to = axis->GetXmax();
   Axis_t width = (to - from) / bins;
   Axis_t *new_bins = new Axis_t[bins + 1];
   for (int i = 0; i <= bins; i++) {
      new_bins[i] = TMath::Power(10, from + i * width);
      if(new_bins[i] > 1e15) {
         std::cout << "The new bin range is huge, better check what you are doing! "<< new_bins[i] << std::endl;
      }
      // std::cout << i << "  " << new_bins[i] << std::endl;
   }
   
   axis->Set(bins, new_bins);
   // std::cout << "New XMin" <<    axis->GetXmin() << std::endl;
   delete new_bins;
} 



void splitGhostrate(){
   
   
   
   /**********************************************************************************************/
   /*               Steering                                                                     */
   /**********************************************************************************************/
   
   //---------- Paths to load and save:
      
   const string MYPATH = "./";                                  // the current path
   std::vector< std::string > LOAD_FILE_NAMES;                    // the root files to be loaded
//    LOAD_FILE_NAMES.push_back( MYPATH + "Feedback.root" );
   LOAD_FILE_NAMES.push_back( MYPATH + "FeedbackForward.root" );
   LOAD_FILE_NAMES.push_back( MYPATH + "FeedbackSilicon.root" );
   LOAD_FILE_NAMES.push_back( MYPATH + "FeedbackSubset.root" );
   
   std::vector< std::string > LOAD_FILE_MEANINGS;
   LOAD_FILE_MEANINGS.push_back( "ForwardTracking" );
   LOAD_FILE_MEANINGS.push_back( "SiliconTracking" );
   LOAD_FILE_MEANINGS.push_back( "TrackSubsetProcessor" );
   
   const string TREENAME = "recoTracks";                                // name of the tree
   const string PICTURE_NAME = "Ghostrate_split";
   const string PICTURE_ENDING = ".svg";
   
   
   

   
   //---------- Values for the histograms:
   int nBins = 20;
   double xMin = 0.1;
   double xMax = 100;
   double xMinLog10 = log10( xMin ); // as we want a logartihmic scale with evenly binning we have to set the min and max accordingly( see http://root.cern.ch/root/roottalk/roottalk06/1213.html )
   double xMaxLog10 = log10( xMax );
   double markerSize = 1.;
   
   
   
   
   
   /**********************************************************************************************/
   /*               The processing of the data                                                   */
   /**********************************************************************************************/
   
   for( unsigned i=0; i < LOAD_FILE_NAMES.size(); i++ ){
      
      
      
      //---------- Optical settings:
      
      gROOT->SetStyle("Plain");    // a style using white instead of this horrible grey
      TCanvas* myCanvas = new TCanvas("myCanvas", "myCanvas", 0, 0, 600, 400);
      myCanvas->SetLogx();
      TLegend* legend = new TLegend( 0.6, 0.4, 0.9, 0.55 );
      legend->SetFillColor( kWhite );
      
      
      
      
      std::string LOAD_FILE_NAME = LOAD_FILE_NAMES[i];
      std::string LOAD_FILE_MEANING = LOAD_FILE_MEANINGS[i];
      
      std::cout<< "loading " << LOAD_FILE_NAME << "\n";
      TFile* datafile = new TFile(LOAD_FILE_NAME.c_str());
      TTree* datatree = datafile->Get(TREENAME.c_str());
      
      
      TH1D *histAll = new TH1D("histAll","",nBins, xMinLog10, xMaxLog10);
      TH1D *histGhost = new TH1D("histGhost","",nBins, xMinLog10, xMaxLog10); 
      TH1D *histContamination = new TH1D("histContamination","",nBins, xMinLog10, xMaxLog10);
      TH1D *histNoContamination = new TH1D("histNoContamination","",nBins, xMinLog10, xMaxLog10);
      
      
      binLogX( histAll, "X" );
      binLogX( histGhost, "X" );
      binLogX( histContamination, "X" );
      binLogX( histNoContamination, "X" );
      
      //---------- linking the branch values to our local variables:
      
      double pT;
      int nTrueTracks;
      TrackType type;
      
      datatree->SetBranchAddress( "pT" , &pT );
      datatree->SetBranchAddress( "nTrueTracks" , &nTrueTracks );
      datatree->SetBranchAddress( "Type" , &type );
      
      
      
      int nEntries = datatree->GetEntries();       //numbers of entries in the tree
      std::cout<< "There are " << nEntries << " entries\n";
      
      //--------- Fill the histograms:
      
      for (int j = 0; j< nEntries; j++){
         
         datatree->GetEntry(j);
         
         histAll->Fill(pT);
//          std::cout << type << "\n";
         
         if( type == GHOST ) histGhost->Fill( pT );
         if( ( type == COMPLETE ) || ( type == INCOMPLETE ) ) histNoContamination->Fill( pT );
         if( ( type == COMPLETE_PLUS ) || ( type == INCOMPLETE_PLUS ) ) histContamination->Fill( pT );
         
         
         
      }
      
      
      
      //--------- Combine two histograms to make an efficiency like plot
      
      TGraphAsymmErrors* graphGhost = new TGraphAsymmErrors( histGhost, histAll );
      graphGhost->SetMarkerColor( 46 );
      graphGhost->SetMarkerStyle( 20 );
      graphGhost->SetMarkerSize( markerSize );
      graphGhost->SetLineColor( 46 );
      legend->AddEntry( graphGhost, "Ghosts" );
      
      
      TGraphAsymmErrors* graphContaminated = new TGraphAsymmErrors( histContamination, histAll );
      graphContaminated->SetMarkerColor( 9 );
      graphContaminated->SetMarkerStyle( 21 );
      graphContaminated->SetMarkerSize( markerSize );
      graphContaminated->SetLineColor( 9 );
      legend->AddEntry( graphContaminated, "Real, Contaminated" );
      
      
      TGraphAsymmErrors* graphNotContaminated = new TGraphAsymmErrors( histNoContamination, histAll );
      graphNotContaminated->SetMarkerColor( 8 );
      graphNotContaminated->SetMarkerStyle( 22 );
      graphNotContaminated->SetMarkerSize( markerSize );
      graphNotContaminated->SetLineColor( 8 );
      legend->AddEntry( graphNotContaminated, "Real, Not Contaminated" );
      
      
      
      TMultiGraph *mg = new TMultiGraph();
      std::string title = LOAD_FILE_MEANING + ", reconstructed tracks";
      mg->SetTitle( title.c_str() );
      mg->Add( graphGhost );
      mg->Add( graphContaminated );
      mg->Add( graphNotContaminated );
      mg->Draw("AP");
      mg->GetYaxis()->SetRangeUser(0.,1.);
      mg->GetXaxis()->SetTitle( "p_{T}[GeV]" );
      
      
      
      legend->Draw("same");
      
      myCanvas->Update();
      
      string pictureSavePath = MYPATH + PICTURE_NAME + "_" + LOAD_FILE_MEANING.c_str() + PICTURE_ENDING;     // where the image will be saved
      
      myCanvas->SaveAs( pictureSavePath.c_str());    //Save the data to an image file
      
      
      
      
      
      delete myCanvas;

   }

   

   
   
   
   
   
   
}