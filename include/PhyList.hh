#ifndef PHYSICS_LIST_HH
#define PHYSICS_LIST_HH 1

#include "ScanGlobals.hh"
#include "G4VUserPhysicsList.hh"
#include "G4VModularPhysicsList.hh"

class PhyList : public G4VUserPhysicsList {
public:
	PhyList();
	~PhyList();

protected:
	void ConstructParticle();
	void ConstructProcess();
	void SetCuts();
};

#endif