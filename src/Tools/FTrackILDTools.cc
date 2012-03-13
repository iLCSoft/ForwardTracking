#include "FTrackILDTools.h"

#include <sstream>

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
