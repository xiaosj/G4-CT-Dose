#include "PrimaryGen.hh"

PrimaryGen::PrimaryGen() {
	pGun = new G4ParticleGun(1);
	// pGun->SetParticleDefinition(G4Gamma::Definition());
}

PrimaryGen::~PrimaryGen() {
	delete pGun;
}

void PrimaryGen::GeneratePrimaries(G4Event* event) {
	pGun->GeneratePrimaryVertex(event);
}