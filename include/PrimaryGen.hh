#ifndef PRIMARY_GEN_HH
#define PRIMARY_GEN_HH 1

#include "globals.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4Event.hh"

class PrimaryGen : public G4VUserPrimaryGeneratorAction {
public:
	PrimaryGen();
	virtual ~PrimaryGen();
	virtual void GeneratePrimaries(G4Event*);

private:
	G4ParticleGun* pGun;
};

#endif