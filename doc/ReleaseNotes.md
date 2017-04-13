# v01-11

* 2017-03-31 Frank Gaede ([PR#5](https://github.com/iLCSoft/ForwardTracking/pull/5))
  - fix all warnings on clang, MacOS 
      - does not include -WeffC++

* 2017-03-31 Andre Sailer ([PR#4](https://github.com/iLCSoft/ForwardTracking/pull/4))
  - Rename DDForwardTracking to SiliconEndcapTracking, solves #3 
  - SiliconEndcapTracking: fix warnings
  - Ignore warnings from external headers

* 2017-03-31 Frank Gaede ([PR#2](https://github.com/iLCSoft/ForwardTracking/pull/2))
  - replace Gear with DD4hep/DDRec
      - using DD4hep::DDRec::ZDiskPetalsData as replacement for
         gear::FTDParameters
      - use DDSurfaces::Surface and DDRec::SurfaceManager as
         replacement for gear::MeasurementSurfaceStore
      - changed indentation for FTDBackgroundProcessor.cc

# v01-11

* 2017-03-31 Frank Gaede ([PR#5](https://github.com/iLCSoft/ForwardTracking/pull/5))
  - fix all warnings on clang, MacOS 
      - does not include -WeffC++

* 2017-03-31 Andre Sailer ([PR#4](https://github.com/iLCSoft/ForwardTracking/pull/4))
  - Rename DDForwardTracking to SiliconEndcapTracking, solves #3 
  - SiliconEndcapTracking: fix warnings
  - Ignore warnings from external headers

* 2017-03-31 Frank Gaede ([PR#2](https://github.com/iLCSoft/ForwardTracking/pull/2))
  - replace Gear with DD4hep/DDRec
      - using DD4hep::DDRec::ZDiskPetalsData as replacement for
         gear::FTDParameters
      - use DDSurfaces::Surface and DDRec::SurfaceManager as
         replacement for gear::MeasurementSurfaceStore
      - changed indentation for FTDBackgroundProcessor.cc

# v01-11
- moved code to GitHub

Oleksandr Viazlo 2017-03-21 
  - Replace ILDCellID0 with LCTrackerCellID

Marko Petric 2017-03-23 
  - Add install statement to CI
  - Add CONTRIBUTING.md and PULL_REQUEST_TEMPLATE and fix test script

Frank Gaede 2017-03-21 
  - add AUTHORS file

Marko Petric 2017-03-20 
  - fix markdown
  - Enable CI and add LICENCE

# v01-10
F.Gaede
* made compatible with c++11
* removed -ansi -pedantic -Wno-long-long
* fixed narrowing in initializer lists

# v01-09
Rosa Simoniello
 * added DDForwardTracking
 * ATT: Endcap* and Sector* files for CLIC temporary added in this package instead of KiTrack till the pattern recognition for CLIC is stable. DDForwardTracking added. It contains the patter recognition for CLIC in the endcap vertex+tracker region. It is interfaced with DD4hep.

# v01-08
 * modified ForwardTracking (F.Gaede): 
 * use a constant map in getOverlapConnectionMap (was filled with uneeded empty hit vectors)
 * added processor parameters: TrackSystemName: KalTest, DDKalTest, ... GetTrackStateAtCaloFace: allow to run w/o Ecal surfaces
 * no change in algorithm
 

# v01-07
* TrueTrackCritAnalyser: added pdg and theta angle to root output
* TrackingFeedbackProcessor: 
  * added possibility to use only hits from different layers for the nHit cut
  * added cut for theta
  * added chi2, Ndf and chi2prob to the reco tracks in the root output
  * added a cut for failed tracks
* ForwardTracking: 
  * added a new quality indicator, especially for the simple subset and 3-hit tracks
  * changed lastLayerToIP to 5
  * changed the > in the chi2prob cut to >=
 
# v01-06-01
* Updated for new lcio TrackState copy constructor taking const reference.

# v01-06
* TrackingFeedbackProcessor: made the minimum rate of true hits in reco track steerable
* ForwardTracking: Got rid of memory leak

# v01-05
* ForwardTracking: more debug output statements
* TrueTrackCritAnalyser: Adapted to changed hit order in Segments
* TrackingFeedbackProcessor: Added the event number and the sum of tracks belonging to a true track to the root file.
* TrackingFeedbackProcessor: 
  * Added output of theta and nHits of true tracks to root output (so one can plot efficiency over theta or nHits)
  * Added output of mcp vertex position to root file
* ForwardTracking: 
  * Added a HelixFit before the Kalman Fit to increase the speed
  * Nested the CA inside a while loop, repeating it with tightened cut until not too many connections are made. Also added the steering parameters for that
* TrueTrackCritAnalyser: Added saving of chi2, Ndf and chi2/Ndf from helix fit to root file

# v01-04 
* ForwardTracking: As FTDTrack now only takes IFTDHit pointers, the passed arguments are changed
* Added FTDBackgroundProcessor which adds salt and pepper background to the FTD TrackerHit collections
* Erased the executables worker and worker_time as they were outdated
* Added executable param_runner, to rerun Marlin several times while altering a steering parameter
* Added executable param_runner_background a version of param_runner specialised for the FTDBackgroundProcessor
* TrueTrackCritAnalyser: 
  * no more fitting is done, tracks are assumed to be already fitted 
  * chi2prob is now stored for the criteria branches as well
* wrote overview for the doxygen
* switched to new subset classes (SubsetHopfieldNN and SubsetSimple)
* TrueTrackCritAnalyser:
  * added output of the distances of 2 hits in the tracks
  * renamed the cut steer parameters to start with Cut
  * added doxygen documentation
* TrackingFeedback:
  * added new class RecoTrack and reworked TrueTrack as well (now every true track gets a TrueTrack object and every reconstructed track gets a RecoTrack object. In the checking they are then linked: RecoTracks now their assigned TrueTracks and vice versa )
  * added output to a root file
  * renamed the cut steer parameters to start with Cut
  * added doxygen documentation
* Added the folder rootscripts containing the root scripts I use to make graphs


# v01-03
* Splitted ForwardTracking into 3: 
  * the core is now in the package KiTrack
  * an implementation of the ABCs in the core is in the package KiTrackMarlin
  * and ForwardTracking now contains all the processors 
 
 This means: 
* The folder FTrack got renamed to KiTrack and is now in the package KiTrack
* The folder Criteria is now in KiTrack as well
* The folder ILDImpl is now in KiTrackMarlin
* The folder Tools is now in KiTrackMarlin
* Each package has now its namespace: "KiTrack" and "KiTrackMarlin"
 
# v01-02
* added finalising of tracks (final refit, radius of innermost hit, hitnumbers of detectors,... see method ForwardTracking::finaliseTrack )
* modified fitting: now done in one central place by the class Fitter
* changed initialisation of FTDTrack: now a MarlinTrkSystem* has to be passed, which can be used by the track to fit
* Modified to use spacepoints
* Added steering parameter: TakeBestVersionOfTrack
* Added steering parameter for the best track subset
* Use new gear params for FTD (getNSensors)
* patched memory leaks
* some renaming of classes and documentation

# v01-01-01
* lengthenSegments() modified to be able to deal with 0 hits situation
* Made an ITrackSubset Interface and added TrackSubsetSimple
* Added new executable to run Marlin (worker.cc) with different backgrounds
* Added a regulator for all the background (FTDBackgroundProcessor) to easily make it stronger or weaker with just one parameter
* Rewrote the TrackingFeedbackProcessor. Changed the MyRelation struct to a class called TrueTrack and modified it.
* Added the Fitter class, which is just a class that fits some hits or a track and returns chi2, chi2prob and Ndf.
* Made the hit connector more specific and thus track finding faster.
* Corrected error in FTDHit01 with the setting of the layer.
* Added more info to StepAnalyser

# v01-01
* added processor to check overlapping of tracks
* added collection flag to write the hits to the LCIO file
* added location AtIP to the TrackStates

# v01-00
* initial import
