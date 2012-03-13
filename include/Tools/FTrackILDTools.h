#ifndef FTrackILDTools_h
#define FTrackILDTools_h

#include <string>

namespace FTrackILD{

/** @return information about the contents of the passed CellID0 */ 
std::string getCellID0Info( int cellID0 );

/** @return the layer given by the cellID0 */
int getCellID0Layer( int cellID0 );


}

#endif