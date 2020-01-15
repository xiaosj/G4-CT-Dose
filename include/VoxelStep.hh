#include "globals.hh"
#include "G4UserSteppingAction.hh"
#include "ScanInfo.hh"

class VoxelStep : public G4UserSteppingAction {
public:
	VoxelStep();
	VoxelStep(ScanInfo* scaninfo);
	virtual ~VoxelStep();
	virtual void UserSteppingAction(const G4Step*);

private:
	ScanInfo* scan;
	G4double minx, miny, minz;
	G4int ctnxy;
};