#include "TrackingFeedbackProcessor.h"
#include <iostream>

#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>

// ----- include for verbosity dependend logging ---------
#include "marlin/VerbosityLevels.h"

#include <EVENT/LCRelation.h>
#include <EVENT/Track.h>
#include <EVENT/MCParticle.h>
#include <cmath>
#include "TVector3.h"
#include "fstream"
#include "SimpleCircle.h"
#include <algorithm>

#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"

#include <sstream>
#include <MarlinCED.h>


using namespace lcio ;
using namespace marlin ;
using namespace FTrack;

std::string intToString (int i){
 
   std::ostringstream sin;
   sin << i;
   return sin.str();
  
}

void setUpRootFile( std::string fileNamePath, std::string treeName){
  
  //std::string fileNamePath = fileNamePath0;
  
  ifstream rf ((fileNamePath + ".root").c_str());       //rf for RootFile
  if (rf) { // The file already exists
   
    int i=0;
    while (rf){         //Try adding a number starting from 1 to the filename until no file with this name exists and use this.
      
      rf.close();
      i++;
      rf.open((fileNamePath + intToString(i) + ".root").c_str());
      
    }
    rename ( (fileNamePath + ".root").c_str() , (fileNamePath + intToString(i) +".root").c_str());      //renames the file in the way,so that our new file can have it's name
    //and not ovrewrite it.
    
  }
  
  
  
  TFile* myRootFile = new TFile((fileNamePath + ".root").c_str(), "RECREATE");        //Make new file, if there is an old one
  TTree* myTree;
  
  myTree = new TTree(treeName.c_str(),"My tree"); //make a new tree
    
  
  
  myTree->Write("",TObject::kOverwrite);
  myRootFile->Close();
  
}


bool hitComp (TrackerHit* i, TrackerHit* j) {
    
   return ( fabs(i->getPosition()[2]) < fabs( j->getPosition()[2]) ); //compare their z values

}



const char* TRACK_TYPE_NAMES[] = {"COMPLETE" , "COMPLETE_PLUS" , "INCOMPLETE" , "INCOMPLETE_PLUS" , "GHOST" , "LOST"}; 
enum TrackType { COMPLETE , COMPLETE_PLUS , INCOMPLETE , INCOMPLETE_PLUS , GHOST , LOST };

struct MyRelation{
   
   LCRelation* lcRelation;
   bool isLost;
   bool isFoundCompletely;
   std::map<Track*,TrackType> relatedTracks;
   
   MyRelation( LCRelation* rel ){isLost = true; isFoundCompletely = false; relatedTracks.clear(); lcRelation = rel;};
};




TrackingFeedbackProcessor aTrackingFeedbackProcessor ;


TrackingFeedbackProcessor::TrackingFeedbackProcessor() : Processor("TrackingFeedbackProcessor") {

    // modify processor description
   _description = "TrackingFeedbackProcessor gives feedback about the Track Search" ;


    // register steering parameters: name, description, class-variable, default value

   registerInputCollection(LCIO::TRACK,
                           "AutTrkCollection",
                           "Name of Cellular Automaton Track output collection",
                           _AutTrkCollection,
                           std::string("AutTracks")); 
   
   
   registerInputCollection(LCIO::LCRELATION,
                           "MCTrueTrackRelCollectionName",
                           "Name of the TrueTrack MC Relation collection",
                           _colNameMCTrueTracksRel,
                           std::string("TrueTracksMCP"));
   
   registerProcessorParameter("RootFileName",
                              "Name of the root file for saving the results (without .root) ",
                              _rootFileName,
                              std::string("FTrackFeedback") );
      
      
   
   
   

}



void TrackingFeedbackProcessor::init() { 

    streamlog_out(DEBUG) << "   init called  " << std::endl ;

    // usually a good idea to
    printParameters() ;
    
    _treeName = "trackCands";
    setUpRootFile(_rootFileName, _treeName);      //prepare the root file.

    _nRun = 0 ;
    _nEvt = 0 ;
    
//      MarlinCED::init(this) ;

}


void TrackingFeedbackProcessor::processRunHeader( LCRunHeader* run) { 

    _nRun++ ;
} 



void TrackingFeedbackProcessor::processEvent( LCEvent * evt ) { 


//-----------------------------------------------------------------------
// Reset drawing buffer and START drawing collection

//   MarlinCED::newEvent(this , 0) ; 

//   CEDPickingHandler &pHandler=CEDPickingHandler::getInstance();

//   pHandler.update(evt); 

//-----------------------------------------------------------------------

   
   ////////////////////////////////////////////////////////////////
   //
   //getting feedback data of the tracks
   //
   //////////////////////////////////////////////////////////////
   
     
   
   int nComplete = 0;         // complete tracks without extra points
   int nCompletePlus = 0;     // complete tracks with extra points
   int nLost = 0;             // lost tracks = tracks that do exist in reality (mcp), but are not represented in the tracksearch results  
   int nIncomplete = 0;       // incomplete tracks without extra points. i.e.: tracks that are too short (for example 1 or 2 hits are still missing)
   int nIncompletePlus = 0;   // incomplete tracks with extra points. the reconstructed track belongs to the true track that hold more than half of the points of the track
   int nGhost = 0;            // ghost tracks = tracks that are reconstructed, but don't actually exist. Pure fiction. a ghost track 
                              // is a track, where no real track owns more than half of the tracks hits.
   int nFoundCompletely = 0;      // when a true track is found completely (restored track is a complete or complete plus track)
   
   LCCollection* col = evt->getCollection( _colNameMCTrueTracksRel ) ;
   
   int nMCTracks = col->getNumberOfElements();
   std::vector<MyRelation*> myRelations; 
   
   // fill the vector with the relations
   for( int i=0; i < nMCTracks; i++){
      
      bool isOfInterest = true;  // A bool to hold information wether this track we are looking at is interesting for us at all
                                 // So we might apply different criteria to it.
                                 // For example: if a track is very curly we might not want to consider it at all.
                                 // So when we want to know how high the percentage of reconstructed tracks is, we might
                                 // want to only consider certain true tracks 
      
      LCRelation* rel = dynamic_cast <LCRelation*> (col->getElementAt(i) );
      MCParticle* mcp = dynamic_cast <MCParticle*> (rel->getTo() );
      Track*    track = dynamic_cast <Track*>      (rel->getFrom() );
      
      
      //CED begin

      
//       MarlinCED::drawMCParticle( mcp, true, evt, 2, 1, 0xff000, 10, 3.5 );

 
      //CED end
      
      //////////////////////////////////////////////////////////////////////////////////
      //If distance from origin is not too high      
      double dist = sqrt(mcp->getVertex()[0]*mcp->getVertex()[0] + 
                     mcp->getVertex()[1]*mcp->getVertex()[1] + 
                     mcp->getVertex()[2]*mcp->getVertex()[2] );
      
      

      if ( dist > 220 ) isOfInterest = false;   // exclude point too far away from the origin. Of course we want them reconstructed too,
                                                // but at the moment we are only looking at the points that are reconstructed by a simple
                                                // Cellular Automaton, which uses the point 0 as a point in the track
      //
      //////////////////////////////////////////////////////////////////////////////////
      
      //////////////////////////////////////////////////////////////////////////////////
      //If pt is not too low
      
      double pt = sqrt( mcp->getMomentum()[0]*mcp->getMomentum()[0] + mcp->getMomentum()[1]*mcp->getMomentum()[1] );
      
      if ( pt < 0.2 ) isOfInterest = false;
      //
      //////////////////////////////////////////////////////////////////////////////////
      
      //////////////////////////////////////////////////////////////////////////////////
      //If there are less than 4 hits in the track
      
      if ( track->getTrackerHits().size() < 4 ) isOfInterest = false;
      //
      //////////////////////////////////////////////////////////////////////////////////
      
      if ( isOfInterest ) myRelations.push_back( new MyRelation( rel ) );
      
   }
   
   nMCTracks = myRelations.size();
   
   col = evt->getCollection( _AutTrkCollection ) ;

   
   if( col != NULL ){

      int nTracks = col->getNumberOfElements()  ;

      //TODO: kann man das nicht einfacher gestalten??? eleganter? das sind schon wieder so viele for schleifen
      
      for(int i=0; i< nTracks ; i++){ //over all tracks
         
         Track* track = dynamic_cast <Track*> ( col->getElementAt(i) ); 
         TrackerHitVec hitVec = track->getTrackerHits();
         
         std::vector<MyRelation*> hitRelations; //containing all the true tracks (relations) that correspond to the hits of the track
                                                //if for example a track consists of 3 points from one true track and two from another
                                                //at the end this vector will have five entries: 3 times one true track and 3 times the other.
                                                //so at the end, all we have to do is to count them.
         
         for( unsigned int j=0; j < hitVec.size(); j++ ){ //over all hits in the track
            
            for( unsigned int k=0; k < myRelations.size(); k++){ //check all relations if they correspond 
            
               
               Track* trueTrack = dynamic_cast <Track*>  ( myRelations[k]->lcRelation->getFrom() ); //get the true track
         
               if ( find (trueTrack->getTrackerHits().begin() , trueTrack->getTrackerHits().end() , hitVec[j] ) 
                     !=  trueTrack->getTrackerHits().end()) // if the hit is contained in this truetrack
                        hitRelations.push_back( myRelations[k] );//add the track (i.e. its relation) to the vector hitRelations
            }
                    
         } 
         

         // After those two for loops we have the vector hitRelations filled with all the true track (relations) that correspond
         // to our reconstructed track. Ideally this vector would now only consist of the same true track again and again.
         //
         // Before we can check what kind of track we have here, we have to get some data.
         // We wanna know the most represented true track:
         
         int nHitsOneTrack = 0;          //number of most true hits corresponding to ONE true track 
         MyRelation* dominantTrueTrack = NULL;    //the true track most represented in the track 
         
        
         sort ( hitRelations.begin() , hitRelations.end()); //Sorting, so all the same elements are at one place
         
         int n=0;                         // number of hits from one track
         MyRelation* previousRel = NULL;  // used to store the previous relation, so we can compare, if there was a change. at first we don't have
                                          // a previous one, so we set it NULL
         for (unsigned int j=0; j< hitRelations.size(); j++){ 
         
            if ( hitRelations[j] == previousRel ) n++;   //for every repeating element count 1. (that's the reason we sorted before, so we can just count them like this)
            else n = 1; //previous was different --> we start again with 1
            
            previousRel = hitRelations[j];
            
            if (n > nHitsOneTrack){ //we have a new winner (a true track) with (currently) the most hits in this track
                             
               nHitsOneTrack = n;
               dominantTrueTrack = hitRelations[j];
               
            }
            
         }
         
         int nHitsTrack = hitVec.size();   //number of hits of the reconstructed track
         int nHitsTrueTrack = 0;           //number of hits of the true track
         
         if (dominantTrueTrack != NULL ){ 
            Track* trueTrack = dynamic_cast <Track*> ( dominantTrueTrack->lcRelation->getFrom() );
            nHitsTrueTrack = trueTrack->getTrackerHits().size();  
            
         }
                                         
                 
                
         
         //So now we have all the values we need to know to tell, what kind of track we have here.
         
         if ( nHitsOneTrack <= nHitsTrack/2. ){  // less than half the points at maximum correspond to one track, this is not a real track, but
            nGhost++;                           // a ghost track
                     
         }
         else{                                   // more than half of the points form a true track!
            
            TrackType trackType;
            
           
            
            if (nHitsOneTrack > nHitsTrueTrack/2.) //If more than half of the points from the true track are covered
               dominantTrueTrack->isLost = false;  // this is guaranteed no lost track  
            
            if (nHitsOneTrack < nHitsTrueTrack)    // there are too few good hits,, something is missing --> incomplete
               
               if (nHitsOneTrack < nHitsTrack){       // besides the hits from the true track there are also additional ones-->
                  nIncompletePlus++;                  // incomplete with extra points
                  trackType = INCOMPLETE_PLUS;
               }   
               else{                                   // the hits from the true track fill the entire track, so its an
                  nIncomplete++;                      // incomplete with no extra points
                  trackType = INCOMPLETE;
               }
               
            else{                                   // there are as many good hits as there are hits in the true track
                                                   // i.e. the true track is represented entirely in this track
               dominantTrueTrack->isFoundCompletely = true;
               
               
               if (nHitsOneTrack < nHitsTrack){       // there are still additional hits stored in the track, it's a
                  nCompletePlus++;                    // complete track with extra points
                  trackType = COMPLETE_PLUS;
               }
               else{                                   // there are no additional points, finally, this is the perfect
                  nComplete++;                        // complete track
                  trackType= COMPLETE;
               }
            }
               
               //we want the true track to know all reconstructed tracks containing him
               dominantTrueTrack->relatedTracks.insert( std::pair<Track*, TrackType> ( track ,trackType ) );  
               
         }   
         
               
      }
      
      // So now we checked all the values. But really all?
      // No, there is a little value that resisted us so far:
      // The lost tracks. We couldn't raise the value nLost on the go because we could only see which ones are not lost
      // and there is no 1:1 relationship.
      // But therefore we used the struct MyRelation and marked all of their isLost booleans
      // to false if we found a corresponding track.
      // so all we have to do is count how many with isLost == true are left.
      
      for( unsigned int i=0; i < myRelations.size(); i++){
         if ( myRelations[i]->isLost == true ) nLost++;
         if ( myRelations[i]->isFoundCompletely ==true ) nFoundCompletely++;
      }
                  
      
      
      ///////////////////////////
      //ouput the data:
      
      std::cout.precision (8);
      
      
      for( unsigned int i=0; i < myRelations.size(); i++){
         
         MCParticle* mcp = dynamic_cast <MCParticle*> (myRelations[i]->lcRelation->getTo());
         Track* cheatTrack = dynamic_cast<Track*>  (myRelations[i]->lcRelation->getFrom());      
         
         double mcpPt = sqrt( mcp->getMomentum()[0]*mcp->getMomentum()[0] + mcp->getMomentum()[1]*mcp->getMomentum()[1] );    
         double mcpP= sqrt( mcpPt*mcpPt + mcp->getMomentum()[2]*mcp->getMomentum()[2] );
         
         
         std::cout << "\n\n\nTrue Track" << i << ": p= " << mcpP << "  pt= " << mcpPt << std::endl;
         std::cout << "px= " << mcp->getMomentum()[0] << " py= " <<  mcp->getMomentum()[1] << " pz= " <<  mcp->getMomentum()[2] << std::endl;
         std::cout << "PDG= " << mcp->getPDG() << std::endl;
         std::cout << "x= " << mcp->getVertex()[0] << " y= " <<  mcp->getVertex()[1] << " z= " <<  mcp->getVertex()[2] << std::endl;
         std::cout << "/gun/direction " << mcp->getMomentum()[0]/mcpP << " " <<  mcp->getMomentum()[1]/mcpP << " " <<  mcp->getMomentum()[2]/mcpP << std::endl;
         
         std::vector<TrackerHit*> cheatTrackHits = cheatTrack->getTrackerHits(); // we write it into an own vector so wen can sort it
         sort (cheatTrackHits.begin() , cheatTrackHits.end() , hitComp );
         
         for (unsigned int j=0; j< cheatTrackHits.size(); j++){
                        
            std::cout  << "\n( "  
                  << cheatTrackHits[j]->getPosition()[0] << " , " 
                  << cheatTrackHits[j]->getPosition()[1] << " , " 
                  << cheatTrackHits[j]->getPosition()[2] << " ) "
                  << cheatTrackHits[j]->getType() ;
            
            // check the angle between the segments
            if ( j < cheatTrackHits.size()-1 ){ //if there is a hit after
               
               TVector3 a( 0.,0.,0. );
               
               if ( j>0 ) a = TVector3( cheatTrackHits[j-1]->getPosition() );
               TVector3 b( cheatTrackHits[j]->getPosition() );
               TVector3 c( cheatTrackHits[j+1]->getPosition() );
               
               TVector3 f = b-a;
               TVector3 g = c-b;
               
               double angle = f.Angle( g ) *180. / M_PI;
               std::cout << "\n angle= " << angle << "°";
               
               
               double anglePhi= f.Phi()-g.Phi(); //the angle between them in the xy plane 
               anglePhi -= 2*M_PI*floor( anglePhi /2. /M_PI );    //to the range from 0 to 2pi 
               if (anglePhi > M_PI) anglePhi -= 2*M_PI;                                   //to the range from -pi to pi
               anglePhi *= 180./M_PI;
               
               std::cout << "\n anglePhi= " << anglePhi << "°";
               
               
               //Check R
               double x=0.;
               double y=0.;
               
               double x1 = a.X();
               double y1 = a.Y();
               double x2 = b.X();
               double y2 = b.Y();
               double x3 = c.X();
               double y3 = c.Y();
               
               //TODO vertikal abfrage: ob x2 == x1. wenn ja dann punkte vertauschen
               
               double ma = (y2-y1)/(x2-x1);
               double mb = (y3-y2)/(x3-x2);
               
               double R=0.;
               
               if ( fabs(mb - ma) > 0.0000001){ 
               
                  x = ( ma*mb*(y1-y3) + mb*(x1+x2) - ma*(x2+x3) )/( 2.*(mb-ma));
                  y = (-1./ma) * ( x - (x1+x2)/2. ) + (y1+y2)/2;
               
                  R = sqrt (( x1 - x )*( x1 - x ) + ( y1 - y )*( y1 - y ));
                  
                  
               }
               
               
               std::cout << "\n R= " << R;
               
               //distance from circle to origin
               double distTo0 = fabs ( R - sqrt( x*x + y*y) );
               std::cout << "\n Distance to origin = " << distTo0;
               
               //
               TVector3 u ( -x , -y , 0.);
               TVector3 v ( cheatTrackHits[j]->getPosition()[0] - x, cheatTrackHits[j]->getPosition()[1] - y , cheatTrackHits[j]->getPosition()[2]);
               if ( j>0 ) u = TVector3 ( cheatTrackHits[j-1]->getPosition()[0] - x, cheatTrackHits[j-1]->getPosition()[1] - y , cheatTrackHits[j-1]->getPosition()[2]);
               
               double zDist = fabs( u.Z() - v.Z() );
                              
//                std::cout << "  ; Winkelweite " << u.Angle(v);
               std::cout << "\n Winkelweite/z " << ( u.Phi() - v.Phi() )/zDist;
               
               
                
            }
            
            
            if ( j >= 2 ){ //if there are 3 points before
               
               double x1 = 0.;
               double y1 = 0.;
               double z1 = 0.;
               
               if ( j > 2){ //we don't need point 0
               
                  x1 = cheatTrackHits[j-3]->getPosition()[0];
                  y1 = cheatTrackHits[j-3]->getPosition()[1];
                  z1 = cheatTrackHits[j-3]->getPosition()[2];
               
               }
               
               double x2 = cheatTrackHits[j-2]->getPosition()[0];
               double y2 = cheatTrackHits[j-2]->getPosition()[1];
               double z2 = cheatTrackHits[j-2]->getPosition()[2];
               
               double x3 = cheatTrackHits[j-1]->getPosition()[0];
               double y3 = cheatTrackHits[j-1]->getPosition()[1];
               double z3 = cheatTrackHits[j-1]->getPosition()[2];
               
               SimpleCircle circle ( x1 , y1 , x2 , y2 , x3 , y3 );
               
               double centerX = circle.getCenterX();
               double centerY = circle.getCenterY();
               double R = circle.getRadius();
               
               TVector3 v ( x2 - centerX , y2 - centerY , z2 );
               TVector3 w ( x3 - centerX , y3 - centerY , z3 );
               
                  
               double deltaPhiPrev = w.Phi() - v.Phi(); //angle in xy plane from center of circle, between point 2 and 3
                  
                  // use this angle and the distance to the next layer to extrapolate
               double zDistPrevLayer = fabs( z3 - z2 );
               double zDistNextLayer = fabs( cheatTrackHits[j]->getPosition()[2] - z3 );
                  
               double deltaPhiNext = deltaPhiPrev * zDistNextLayer / zDistPrevLayer;
                  
               double phiNext = w.Phi() + deltaPhiNext;
                  
               double xNext = centerX + R* cos(phiNext);
               double yNext = centerY + R* sin(phiNext);
               
               double xTrue = cheatTrackHits[j]->getPosition()[0];
               double yTrue = cheatTrackHits[j]->getPosition()[1];
               
               double distToPrediction = sqrt ( ( xNext- xTrue )*( xNext- xTrue ) + ( yNext- yTrue )*( yNext- yTrue ) );
               double distNormed = distToPrediction / zDistNextLayer;   
               
               std::cout << "\n  ; Prediction (x,y)= (" << xNext << " , " << yNext << ")";
               std::cout << " ; distance normed= " << distNormed;
               
            }
            
            //check the ratio of the distance between to points and their z distance (= criteria for building the segments)
            
            if (j>=0){ 
            
               TVector3 a( 0.,0.,0. );
               if ( j>0 ) a = TVector3( cheatTrack->getTrackerHits()[j-1]->getPosition() );
               TVector3 b( cheatTrack->getTrackerHits()[j]->getPosition() );
               TVector3 dist = a - b; // vector between the the two hits
                              
               double distance = dist.Mag(); //the distance between the hits
               dist.SetX(0.);
               dist.SetY(0.);
               double zDistance = dist.Mag(); //the z distance
               double ratio = distance / zDistance;
               
               std::cout << "\n Dist/xDist= " << ratio << " ";
               
            }
            
            std::cout << std::endl;
            
         }
         
         
         if (myRelations[i]->isLost == true) std::cout << "LOST" <<std::endl;
         if (myRelations[i]->isFoundCompletely== false) std::cout << "NOT FOUND COMPLETELY";
                    
         std::map<Track*,TrackType> relTracks = myRelations[i]->relatedTracks;
         for (std::map<Track*,TrackType>::const_iterator it = relTracks.begin(); it != relTracks.end(); ++it)
         {
            std::cout << TRACK_TYPE_NAMES[it->second] << "\t";
            
            for (unsigned int k = 0; k < it->first->getTrackerHits().size(); k++){
               
               std::cout << "(" << it->first->getTrackerHits()[k]->getPosition()[0] << ","
                     << it->first->getTrackerHits()[k]->getPosition()[1] << ","
                     << it->first->getTrackerHits()[k]->getPosition()[2] << ")";
               
            }
            
            std::cout<<"\n";
         }

      }
      
      
      //The statistics:
      
      float pLost=-1.;
      float pGhost=-1.; 
      float pComplete=-1.;
      float pFoundCompletely=-1.;
      
      if (nMCTracks > 0) pLost = 100.* nLost/nMCTracks;           //percentage of true tracks that are lost

      if (nTracks > 0) pGhost = 100.* nGhost/nTracks;             //percentage of found tracks that are ghosts
      
      if (nMCTracks > 0) pComplete = 100. * nComplete/nMCTracks;  //percentage of true tracks that where found complete with no extra points
   
      if (nMCTracks > 0) pFoundCompletely = 100. * nFoundCompletely/nMCTracks;
      
      std::cout << std::endl;
      std::cout << std::endl;
      std::cout << "nMCTracks = " << nMCTracks <<std::endl;
      std::cout << "nTracks = " << nTracks <<std::endl;
      std::cout << "nFoundCompletely = " << nFoundCompletely << " , " << pFoundCompletely << "%" <<std::endl;
      std::cout << "nLost = " << nLost << " , " << pLost << "%" << std::endl;
      std::cout << "nGhost = " << nGhost << " , " << pGhost << "%" << std::endl;   
      
      std::cout << "nComplete = " << nComplete << " , " << pComplete << "%" <<std::endl;
      std::cout << "nCompletePlus = " << nCompletePlus <<std::endl;
      
      std::cout << "nIncomplete = " << nIncomplete <<std::endl;
      std::cout << "nIncompletePlus = " << nIncompletePlus <<std::endl;
      
      std::cout << std::endl;
      
      
      std::cout << std::endl;
      std::cout << std::endl;
      
      ////
      
      ofstream myfile;
      myfile.open ("Feedback.csv" , std::ios::app);
      
      if (isFirstEvent()) myfile << std::endl;

      
      
      myfile << std::endl;
      myfile << "nFoundCompletely\t" << nFoundCompletely << "\t\t" << pFoundCompletely << "\t%\t\t";
      myfile << "nLost\t" << nLost << "\t\t" << pLost << "\t%" << "\t\t";
      myfile << "nGhost\t" << nGhost << "\t\t" << pGhost << "\t%" << "\t\t"; 
      
      myfile << "nMCTracks\t" << nMCTracks <<"\t\t";
      myfile << "nTracks\t" << nTracks <<"\t\t";
      
      myfile << "nComplete\t" << nComplete << "\t" << pComplete << "%\t\t";
      myfile << "nCompletePlus\t" << nCompletePlus <<"\t\t\t";
      
      myfile << "nIncomplete\t" << nIncomplete <<"\t\t\t";
      myfile << "nIncompletePlus\t" << nIncompletePlus <<"\t\t\t";
      
      myfile.close();

      
      //Save it to a root file

      std::map < std::string , int > rootData;
      std::map < std::string , int >::iterator it;

            
      rootData.insert ( std::pair < std::string , int> ( "nMCTracks" ,          nMCTracks ) );
      rootData.insert ( std::pair < std::string , int> ( "nRecoTracks" ,        nTracks ) );
      rootData.insert ( std::pair < std::string , int> ( "nFoundCompletely" ,   nFoundCompletely ) );
      rootData.insert ( std::pair < std::string , int> ( "nLost" ,              nLost ) );
      rootData.insert ( std::pair < std::string , int> ( "nGhost" ,             nGhost ) );
      rootData.insert ( std::pair < std::string , int> ( "nComplete" ,          nComplete ) );
      rootData.insert ( std::pair < std::string , int> ( "nCompletePlus" ,      nCompletePlus ) );
      rootData.insert ( std::pair < std::string , int> ( "nIncomplete" ,        nIncomplete ) );
      rootData.insert ( std::pair < std::string , int> ( "nIncompletePlus" ,    nIncompletePlus ) );
      
        
      std::map < std::string , double > rootDataDouble;
      std::map < std::string , double >::iterator itD;
      
      rootDataDouble.insert ( std::pair < std::string , double> ( "pFoundCompletely" , pFoundCompletely ) );
      rootDataDouble.insert ( std::pair < std::string , double> ( "pLost" , pLost ) );
      rootDataDouble.insert ( std::pair < std::string , double> ( "pGhost" , pGhost ) );
      rootDataDouble.insert ( std::pair < std::string , double> ( "pComplete" , pComplete ) );
      
      
      TFile*   myRootFile = new TFile( (_rootFileName + ".root" ).c_str(), "UPDATE"); //add values to the root file
      TTree*   myTree = (TTree*) myRootFile->Get(_treeName.c_str());
      
      
      
      
      for( it = rootData.begin() ; it != rootData.end() ; it++){
 
         
         if ( isFirstEvent() )  myTree->Branch( (*it).first.c_str(), & (*it).second  );
            
         else myTree->SetBranchAddress( (*it).first.c_str(), & (*it).second );   
               
         
      }
      
      
      for( itD = rootDataDouble.begin() ; itD != rootDataDouble.end() ; itD++){
 
        
         if ( isFirstEvent() ) myTree->Branch( (*itD).first.c_str(), & (*itD).second  ); 
       
         else myTree->SetBranchAddress( (*itD).first.c_str(), & (*itD).second );  
         
         
      }
      
      
      myTree->Fill();
      myTree->Write("",TObject::kOverwrite);
      
      myRootFile->Close();

      ////
   }
   
   
    //-- note: this will not be printed if compiled w/o MARLINDEBUG=1 !

    streamlog_out(DEBUG) << "   processing event: " << evt->getEventNumber() 
        << "   in run:  " << evt->getRunNumber() << std::endl ;


//      MarlinCED::draw(this); //CED
        
    _nEvt ++ ;
}



void TrackingFeedbackProcessor::check( LCEvent * evt ) { 
    // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void TrackingFeedbackProcessor::end(){ 

    //   std::cout << "MyProcessor::end()  " << name() 
    // 	    << " processed " << _nEvt << " events in " << _nRun << " runs "
    // 	    << std::endl ;

}

