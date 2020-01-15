#ifndef VOXEL_NESTEDPARAM_HH
#define VOXEL_NESTEDPARAM_HH 1

#include "ScanGlobals.hh"
#include "ScanInfo.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VNestedParameterisation.hh"

class VoxelNestedParam : public G4VNestedParameterisation {
public:
	VoxelNestedParam();
	VoxelNestedParam(ScanInfo* scaninfo);
	~VoxelNestedParam();

	// Pure virtual functions must be implemented
	G4Material* ComputeMaterial(G4VPhysicalVolume *currentVol,
                                const G4int repNo,
                                const G4VTouchable *parentTouch);
    G4int       GetNumberOfMaterials() const;
    G4Material* GetMaterial(G4int idx) const;
    void ComputeTransformation(const G4int no, G4VPhysicalVolume *currentPV) const;

private:
	ScanInfo* scan;
	G4int ctnxy;
	G4double* trans_y;
};

#endif