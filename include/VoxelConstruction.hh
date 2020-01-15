#ifndef VOXEL_CONSTRUCTION_HH
#define VOXEL_CONSTRUCTION_HH 1

#include "ScanGlobals.hh"
#include "ScanInfo.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4VPhysicalVolume.hh"

class VoxelConstruction : public G4VUserDetectorConstruction {
public:
	VoxelConstruction();
	VoxelConstruction(ScanInfo* scaninfo);
	~VoxelConstruction();
	G4VPhysicalVolume* Construct();

private:
	ScanInfo* scan;
};

#endif