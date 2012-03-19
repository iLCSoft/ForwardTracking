#include "FTrackILDTools.h"

#include <sstream>
#include <fstream>
#include <cmath>

#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"

#include <UTIL/ILDConf.h>


std::string FTrackILD::getCellID0Info( int cellID0 ){
   
   std::stringstream s;
   
   //find out layer, module, sensor
   UTIL::BitField64  cellID( UTIL::ILDCellID0::encoder_string );
   cellID.setValue( cellID0 );
   
   int subdet = cellID[ UTIL::ILDCellID0::subdet ] ;
   int side   = cellID[ UTIL::ILDCellID0::side ];
   int module = cellID[ UTIL::ILDCellID0::module ];
   int sensor = cellID[ UTIL::ILDCellID0::sensor ];
   int layer  = cellID[ UTIL::ILDCellID0::layer ];
   
   s << "(su" << subdet << ",si" << side << ",la" << layer << ",mo" << module << ",se" << sensor << ")";
   
   return s.str();
   
}

int FTrackILD::getCellID0Layer( int cellID0 ){
   
   
   UTIL::BitField64  cellID( UTIL::ILDCellID0::encoder_string );
   cellID.setValue( cellID0 );
   
   return cellID[ UTIL::ILDCellID0::layer ];
   
   
}



//TODO: don't assume a fileending of .root. Be more flexible! (Maybe even write a more general routine dealing with any file


void FTrackILD::setUpRootFile( std::string rootNamePath, std::string treeName ,  std::set<std::string> branchNames , bool createNew ){
   
   
   std::string fileNamePath = rootNamePath.substr( 0 , rootNamePath.find_last_of(".")  );
   
   
   
   ifstream rf ((fileNamePath + ".root").c_str());       //rf for RootFile
   if ((rf) && (createNew)) { // The file already exists and we don't want to append
   
    int i=0;
    std::stringstream newname;
    while (rf){         //Try adding a number starting from 1 to the filename until no file with this name exists and use this.
      
      rf.close();
      i++;
      newname.str("");
      newname << fileNamePath << i << ".root";
      
      rf.open( newname.str().c_str() );
      
    }
    rename ( (fileNamePath + ".root").c_str() , newname.str().c_str());      //renames the file in the way,so that our new file can have it's name
    //and not ovrewrite it.
    
   }
   
   float x = 0;
   
   std::string modus = "RECREATE";
   if ( !createNew ) modus = "UPDATE";
   
   TFile* myRootFile = new TFile((fileNamePath + ".root").c_str(), modus.c_str() );        //Make new file or update it
   TTree* myTree;
   
   myTree = new TTree(treeName.c_str(),"My tree"); //make a new tree
   
   //create the branches:
   
   std::set < std::string >::iterator it;
   
   
   for ( it = branchNames.begin() ; it != branchNames.end() ; it++ ){
      
      
      myTree->Branch( (*it).c_str() , &x );
      
   }
   
   
   myTree->Write("",TObject::kOverwrite);
   myRootFile->Close();
   
}





void FTrackILD::saveToRoot( std::string rootFileName, std::string treeName , std::map < std::string , float > map_name_data ){
   
   
   
   std::map < std::string , float >::iterator it;
   
   
   TFile*   myRootFile = new TFile( rootFileName.c_str(), "UPDATE"); //add values to the root file
   TTree*   myTree = dynamic_cast <TTree*>( myRootFile->Get( treeName.c_str()) );
   
   
   
   
   for( it = map_name_data.begin() ; it != map_name_data.end() ; it++){
      
      
      
      myTree->SetBranchAddress( it->first.c_str(), & it->second );   
      
      
   }
   
   
   myTree->Fill();
   myTree->Write("",TObject::kOverwrite);
   
   myRootFile->Close();
   
}

void FTrackILD::saveToRoot( std::string rootFileName, std::string treeName , std::vector < std::map < std::string , float > > rootDataVec ){
   
   
   
   std::map < std::string , float >::iterator it;
   
   
   TFile*   myRootFile = new TFile( rootFileName.c_str(), "UPDATE"); //add values to the root file
   TTree*   myTree = dynamic_cast <TTree*>( myRootFile->Get( treeName.c_str()) );
   
   
   for( unsigned i=0; i<rootDataVec.size(); i++ ){ //for all entries
   
      
      std::map < std::string , float > map_name_data = rootDataVec[i];
      
      for( it = map_name_data.begin() ; it != map_name_data.end() ; it++){ // for all data in the entrie
         
         
         
         myTree->SetBranchAddress( it->first.c_str(), & it->second );   
         
         
         
      }
      
      
      myTree->Fill();
      
      
   }
   myTree->Write("",TObject::kOverwrite);   
   myRootFile->Close();
   
}


bool FTrackILD::compare_TrackerHit_z( EVENT::TrackerHit* a, EVENT::TrackerHit* b ){
   
   return ( fabs(a->getPosition()[2]) < fabs( b->getPosition()[2]) ); //compare their z values
   
}




FTDHitSimple* FTrackILD::createVirtualIPHit( int side , const SectorSystemFTD* sectorSystemFTD ){
   
   unsigned layer = 0;
   unsigned module = 0;
   unsigned sensor = 0;
   FTDHitSimple* virtualIPHit = new FTDHitSimple( 0.,0.,0., side , layer , module , sensor , sectorSystemFTD );
   
   
   virtualIPHit->setIsVirtual ( true );
   
   return virtualIPHit;
   
}



