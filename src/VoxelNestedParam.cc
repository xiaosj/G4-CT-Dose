#include "VoxelNestedParam.hh"

#include "G4Material.hh"
#include "G4PhantomParameterisation.hh"
#include "G4Box.hh"
#include "G4ThreeVector.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VTouchable.hh"
#include "G4PVParameterised.hh"

VoxelNestedParam::VoxelNestedParam() {}

VoxelNestedParam::VoxelNestedParam(ScanInfo* scaninfo) {
	scan = scaninfo;
    ctnxy = scan->ctnx * scan->ctny;
    trans_y = new G4double[scan->ctny];
    for(int iy = 0; iy < scan->ctny; iy++)
        trans_y[iy] = (2. * iy + 1. - scan->ctny) * scan->ctdy * 0.5;
}

VoxelNestedParam::~VoxelNestedParam() {
    delete [] trans_y;
}


G4Material* VoxelNestedParam::
ComputeMaterial(G4VPhysicalVolume *currentVol,
                const G4int repNo,  // it is iy in the replica
                const G4VTouchable *parentTouch) {
	//
	if(parentTouch == nullptr)
		return scan->mat[0];

	// Copy number of voxels
    G4int iz = parentTouch->GetReplicaNumber(1);
    G4int ix = parentTouch->GetReplicaNumber(0);
    G4int copyNo = ix + scan->ctnx * repNo + ctnxy * iz;
    unsigned int id = scan->matID[copyNo];
    // printf("(%d,%d,%d) mat[%d]\n", ix, repNo, iz, id);
    return scan->mat[id];
}


G4int VoxelNestedParam::GetNumberOfMaterials() const {
	return CT_MATERIALS+1;
}


G4Material* VoxelNestedParam::GetMaterial(G4int idx) const {
	return scan->mat[idx];
}

void VoxelNestedParam::
ComputeTransformation(const G4int copyNo, G4VPhysicalVolume* physVol) const {
    // Position of voxels.
    // x and z positions are already defined in VoxelConstruction by using
    // replicated volume. Here only we need to define is y positions of voxels.
	physVol->SetTranslation(G4ThreeVector(0., trans_y[copyNo], 0.));
}
