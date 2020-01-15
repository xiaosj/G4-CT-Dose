#include "VoxelStep.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4Track.hh"
#include "G4VTouchable.hh"

VoxelStep::VoxelStep() {}

VoxelStep::VoxelStep(ScanInfo* scaninfo) {
	G4double sx, sy, sz;
	scan = scaninfo;
	sx = scan->ctdx * 0.1;
	sy = scan->ctdy * 0.1;
	sz = scan->ctdz * 0.1;
	minx = -scan->ctnx * scan->ctdx * 0.5 + sx;
	miny = -scan->ctny * scan->ctdy * 0.5 + sy;
	minz = -scan->ctnz * scan->ctdz * 0.5 + sz;
	ctnxy = scan->ctnx * scan->ctny;
}

VoxelStep::~VoxelStep() {}

void VoxelStep::UserSteppingAction(const G4Step* theStep) {
	G4int idx;
	G4double dE = theStep->GetTotalEnergyDeposit();
	//G4StepPoint* stepPoint = theStep->GetPostStepPoint();
	//G4VPhysicalVolume* thePV = stepPoint->GetPhysicalVolume();

	G4int ix, iy, iz;
	const G4VTouchable* th;

    G4StepPoint* prePoint = theStep->GetPreStepPoint();
    G4VPhysicalVolume* prePV = prePoint->GetPhysicalVolume();
	/*if(prePV == scan->ctPV) {
	    th = prePoint->GetTouchable();
	    iz = th->GetReplicaNumber(2);
	    ix = th->GetReplicaNumber(1);
	    iy = th->GetReplicaNumber(0);
	    sp = prePoint->GetPosition();
	    printf("from (%d,%d,%d) - (%.3f,%.3f,%.3f) -> ", ix, iy, iz, sp.x(), sp.y(), sp.z());
		ix = (sp.x() - minx) / scan->ctdx;
		iy = (sp.y() - miny) / scan->ctdy;
		iz = (sp.z() - minz) / scan->ctdz;
	    printf("(%d,%d,%d), ", ix, iy, iz);

	    th = stepPoint->GetTouchable();
	    iz = th->GetReplicaNumber(2);
	    ix = th->GetReplicaNumber(1);
	    iy = th->GetReplicaNumber(0);
	    sp = stepPoint->GetPosition();
	    printf("to (%d,%d,%d) - (%.3f,%.3f,%.3f) -> ", ix, iy, iz, sp.x(), sp.y(), sp.z());
		ix = (sp.x() - minx) / scan->ctdx;
		iy = (sp.y() - miny) / scan->ctdy;
		iz = (sp.z() - minz) / scan->ctdz;
	    printf("(%d,%d,%d)\n", ix, iy, iz);
	}

	if(thePV == scan->ctPV) {
	    //printf(" to (%d,%d,%d)\n", ix, iy, iz);		
	}*/

	// if(thePV == scan->ctPV)  {
	// 	idx = thePV->GetCopyNo();
	// 	G4cout << std::fixed << std::setprecision(2)
	// 		   << stepPoint->GetPosition() << ": idx=" << idx << G4endl;
	// 	printf("dE = %.2e\n", dE);
	// }

	if(dE > 0 && prePV == scan->ctPV) {
	    th = prePoint->GetTouchable();
	    iz = th->GetReplicaNumber(2);
	    ix = th->GetReplicaNumber(1);
	    iy = th->GetReplicaNumber(0);
		//G4int ix, iy, iz;
		//sp = stepPoint->GetPosition();
		//ix = (sp.x() - minx) / scan->ctdx;
		//iy = (sp.y() - miny) / scan->ctdy;
		//iz = (sp.z() - minz) / scan->ctdz;
		idx = ix + iy * scan->ctnx + iz * ctnxy;

		// if(idx0 != idx) {
			// printf("idx=%d (%d,%d,%d), idx0=%d\n", idx, ix, iy, iz, idx0);
		// 	printf("  At (%.3f,%.3f,%.3f), idx0 = %d (%d,%d,%d), idx1 = %d (%d,%d,%d)\n",
		//  		sp.x(), sp.y(), sp.z(),
		// 		idx0, ix1, iy1, iz1,
		//  		idx, ix, iy, iz);			
		// }
		// if(idx >= scan->ctnx*scan->ctny*scan->ctnz || idx < 0) {
		// 	printf("idx exceed: %d (%d,%d,%d)=(%.3f,%.3f,%.3f), %d, (%d,%d).\n",
		// 		idx, ix, iy, iz,
		// 		sp.x(), sp.y(), sp.z(),
		// 		idx2, scan->ctnx, nxy);
		// 	idx = scan->ctnx*scan->ctny*scan->ctnz - 1;
		// }
		// printf("      - (%.3f,%.3f,%.3f)=%d vs. %d\n", sp.x(), sp.y(), sp.z(), idx, idx2);		
		// iz = (G4int)(idx / (scan->ctnx * scan->ctny));
		// iy = (G4int)((idx - iz * scan->ctnx * scan->ctny) / scan->ctnx);
		// ix = idx - iz * scan->ctnx * scan->ctny - iy * scan->ctnx;
		// printf("      - %d: %.2f keV, (%d,%d,%d)=(%.3f,%.3f,%.3f), (%.3f,%.3f,%.3f)\n",
		// 	 idx, dE/keV, ix, iy, iz,
		// 	 (ix+0.5)*scan->ctdx+minx, (iy+0.5)*scan->ctdy+miny, (iz+0.5)*scan->ctdz+minz,
		// 	 sp.x(), sp.y(), sp.z());

		// G4Track* theTrack = theStep->GetTrack();
		scan->de[idx] += dE;// * theTrack->GetWeight();

	}
}