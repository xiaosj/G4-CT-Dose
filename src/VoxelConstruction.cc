#include "VoxelConstruction.hh"

#include "G4Material.hh"
#include "G4PhantomParameterisation.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVParameterised.hh"
#include "VoxelNestedParam.hh"

#define WORLD_SIZE 80

VoxelConstruction::VoxelConstruction() {}

VoxelConstruction::VoxelConstruction(ScanInfo* scaninfo) {
	scan = scaninfo;
	Construct();
}


VoxelConstruction::~VoxelConstruction() {}


G4VPhysicalVolume* VoxelConstruction::Construct() {
	//-------------- WORLD --------------
	G4Material* air  = G4Material::GetMaterial("G4_AIR");
	G4Box* worldS = new G4Box("world", WORLD_SIZE*cm, WORLD_SIZE*cm, WORLD_SIZE*cm);
	G4LogicalVolume* worldV = new G4LogicalVolume(worldS, air, "world");
	G4VPhysicalVolume* worldPV = new G4PVPlacement(
		0,					// no rotation
		G4ThreeVector(),	// at (0,0,0)
		worldV,				// its logical volume
		"world",			// its name
		0,					// its mother volume (0: none)
		false,				// no boolean operations
		0);					// its copy number

	//-------------- CT Container -------------
	G4Box* contS = new G4Box(
		"container",
		scan->voxelnx * scan->ctdx * 0.5,
		scan->ctny * scan->ctdy * 0.5,
		scan->voxelnz * scan->ctdz * 0.5);
	G4LogicalVolume* contV = new G4LogicalVolume(contS, air, "container");
	G4VPhysicalVolume* contPV = new G4PVPlacement(
		0,					// no rotation
		G4ThreeVector(),	// at (0,0,0)
		contV,				// its logical volume
		"container",		// its name
		worldV,				// its mother volume (0: none)
		false,				// no boolean operations
		1);					// its copy number

	//--------------- CT Voxel ----------------
	/*
	G4PhantomParameterisation* param = new G4PhantomParameterisation;
	param->SetVoxelDimensions(scan->ctdx*0.5, scan->ctdy*0.5, scan->ctdz*0.5);
	param->SetNoVoxel(scan->ctnx, scan->ctny, scan->ctnz);
	param->SetMaterials(scan->ctMat);
	param->SetMaterialIndices(scan->matID);

	G4Box* voxelS = new G4Box("voxel", scan->ctdx*0.5, scan->ctdy*0.5, scan->ctdz*0.5);
	G4LogicalVolume* voxelV = new G4LogicalVolume(voxelS, scan->ctMat[0], "voxel");

	param->BuildContainerSolid(contPV);	// Assign the container volume of the parameterisation

	// Assure yourself that the voxels are completely filling the container volume
	param->CheckVoxelsFillContainer(contS->GetXHalfLength(), contS->GetYHalfLength(), contS->GetZHalfLength());

	// The G4PVParameterised object that uses the created parameterisation should be placed in the container logical volume
	G4PVParameterised* ctPV = new G4PVParameterised("ct", voxelV, contV, kYAxis,
		scan->ctnx*scan->ctny*scan->ctnz, param);
	// if axis is set as kUndefined instead of kXAxis, GEANT4 will do an smart voxel optimisation (not needed if G4RegularNavigation is used)

	// Set this physical volume as having a regular structure of type 1, so that G4RegularNavigation is used
	ctPV->SetRegularStructureId(1); // if not set, G4VoxelNavigation will be used instead
	*/

	//---------------- CT Voxel from nested replica -------------
	G4VSolid* solRepZ = new G4Box("RepZS",
		scan->voxelnx * scan->ctdx * 0.5,
		scan->ctny * scan->ctdy * 0.5,
		scan->ctdz * 0.5);
	G4LogicalVolume* logRepZ = new G4LogicalVolume(solRepZ, air, "RepZL");
	new G4PVReplica("RepZP", logRepZ, contV, kZAxis, scan->voxelnz, scan->ctdz);

	G4VSolid* solRepX = new G4Box("RepXS",
		scan->ctdx * 0.5,
		scan->ctny * scan->ctdy * 0.5,
		scan->ctdz * 0.5);
	G4LogicalVolume* logRepX = new G4LogicalVolume(solRepX, air, "RepXL");
	new G4PVReplica("RepXP", logRepX, logRepZ, kXAxis, scan->voxelnx, scan->ctdx);

	G4VSolid* solVoxel = new G4Box("voxelS",
		scan->ctdx * 0.5,
		scan->ctdy * 0.5,
		scan->ctdz * 0.5);
	G4LogicalVolume* logVoxel = new G4LogicalVolume(solVoxel, air, "voxelL");

	VoxelNestedParam* param = new VoxelNestedParam(scan);
	G4PVParameterised* ctPV = new G4PVParameterised("ct", logVoxel, logRepX, kUndefined, scan->ctny, param);
	scan->ctPV = ctPV;

	return worldPV;
}