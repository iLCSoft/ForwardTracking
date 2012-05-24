#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"

#include <iostream>
#include <string>
#include <vector>

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



void splitEfficiency(){
   
   
   
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
   
   const string TREENAME = "trueTracks";                                // name of the tree
   const string PICTURE_NAME = "Efficiency_split";
   const string PICTURE_ENDING = ".svg";
   
   
   

   
   //---------- Values for the histograms:
   int nBins = 20;
   double xMin = 0.1;
   double xMax = 100;
   double xMinLog10 = log10( xMin ); // as we want a logartihmic scale with evenly binning we have to set the min and max accordingly( see http://root.cern.ch/root/roottalk/roottalk06/1213.html )
   double xMaxLog10 = log10( xMax );
   double markerSize = 0.8;
   
   
   
   
   
   /**********************************************************************************************/
   /*               The processing of the data                                                   */
   /**********************************************************************************************/
   
   for( unsigned i=0; i < LOAD_FILE_NAMES.size(); i++ ){
      
      
      
      //---------- Optical settings:
      
      gROOT->SetStyle("Plain");    // a style using white instead of this horrible grey
      TCanvas* myCanvas = new TCanvas("myCanvas", "myCanvas", 0, 0, 600, 400);     //"new"-Command ist notwendig, damit die Canvas erhalten bleibt.
      myCanvas->SetLogx();
      TLegend* legend = new TLegend( 0.6, 0.9, 0.9, 0.99 );
      legend->SetFillColor( kWhite );
      
      
      
      
      std::string LOAD_FILE_NAME = LOAD_FILE_NAMES[i];
      std::string LOAD_FILE_MEANING = LOAD_FILE_MEANINGS[i];
      
      std::cout<< "loading " << LOAD_FILE_NAME << "\n";
      TFile* datafile = new TFile(LOAD_FILE_NAME.c_str());
      TTree* datatree = datafile->Get(TREENAME.c_str());
      
      
      TH1D *histAll = new TH1D("histAll","",nBins, xMinLog10, xMaxLog10);
      TH1D *histLost = new TH1D("histFound","",nBins, xMinLog10, xMaxLog10); 
      TH1D *histContamination = new TH1D("histContamination","",nBins, xMinLog10, xMaxLog10);
      TH1D *histNoContamination = new TH1D("histNoContamination","",nBins, xMinLog10, xMaxLog10);
      
      
      binLogX( histAll, "X" );
      binLogX( histLost, "X" );
      binLogX( histContamination, "X" );
      binLogX( histNoContamination, "X" );
      
      //---------- linking the branch values to our local variables:
      
      double pT;
      int nComplete;
      int nCompletePlus;
      int nIncomplete;
      int nIncompletePlus;
      
      
      datatree->SetBranchAddress ("pT" , &pT);
      datatree->SetBranchAddress ("nComplete" , &nComplete);
      datatree->SetBranchAddress ("nCompletePlus" , &nCompletePlus);
      datatree->SetBranchAddress ("nIncomplete" , &nIncomplete);
      datatree->SetBranchAddress ("nIncompletePlus" , &nIncompletePlus);
      
      
      
      int nEntries = datatree->GetEntries();       //numbers of entries in the tree
      std::cout<< "There are " << nEntries << " entries\n";
      
      //--------- Fill the histograms:
      
      for (int j = 0; j< nEntries; j++){
         
         datatree->GetEntry(j);
         
         histAll->Fill(pT);
         
         int nContaminated = nCompletePlus + nIncompletePlus;
         int nNotContaminated = nComplete + nIncomplete;
         
         
         // Decide which kind it belongs to
         
         if( nContaminated == 0 ){
            
            if( nNotContaminated == 0){ // neither contaminated nor incontaminated ones -> this is lost
               
               histLost->Fill( pT );
               
            }
            else{
               
               histNoContamination->Fill( pT );
               
            }
            
         }
         else{
            
            histContamination->Fill( pT );
            
         }
         
         
      }
      
      
      
      //--------- Combine two histograms to make an efficiency like plot
      
      TGraphAsymmErrors* graphLost = new TGraphAsymmErrors( histLost, histAll );
      graphLost->SetMarkerColor( 6 );
      graphLost->SetMarkerStyle( 20 );
      graphLost->SetMarkerSize( markerSize );
      graphLost->SetLineColor( 6 );
      legend->AddEntry( graphLost, "Lost" );
      
      
      TGraphAsymmErrors* graphContaminated = new TGraphAsymmErrors( histContamination, histAll );
      graphContaminated->SetMarkerColor( 7 );
      graphContaminated->SetMarkerStyle( 21 );
      graphContaminated->SetMarkerSize( markerSize );
      graphContaminated->SetLineColor( 7 );
      legend->AddEntry( graphContaminated, "Found, Contaminated" );
      
      
      TGraphAsymmErrors* graphNotContaminated = new TGraphAsymmErrors( histNoContamination, histAll );
      graphNotContaminated->SetMarkerColor( 3 );
      graphNotContaminated->SetMarkerStyle( 22 );
      graphNotContaminated->SetMarkerSize( markerSize );
      graphNotContaminated->SetLineColor( 3 );
      legend->AddEntry( graphNotContaminated, "Found, Not Contaminated" );
      
      
      
      TMultiGraph *mg = new TMultiGraph();
      std::string title = LOAD_FILE_MEANING + ", True Tracks";
      mg->SetTitle( title.c_str() );
      mg->Add( graphLost );
      mg->Add( graphContaminated );
      mg->Add( graphNotContaminated );
      mg->Draw("AP");
      mg->GetYaxis()->SetRangeUser(0.,1.);
      mg->GetXaxis()->SetTitle( "p_{T}[GeV]" );
      
      
      
      legend->Draw("same");
      
      myCanvas->Update();
      
      const string pictureSavePath = MYPATH + PICTURE_NAME + "_" + LOAD_FILE_MEANING.c_str() + PICTURE_ENDING;     // where the image will be saved
      
      myCanvas->SaveAs( pictureSavePath.c_str());    //Save the data to an image file
      
      
      delete myCanvas;
      
   }

   

   
   
   
   
   
   
}