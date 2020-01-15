#include "PhyList.hh"
#include "G4ParticleTypes.hh"
#include "G4ProcessManager.hh"
#include "G4EmStandardPhysics.hh"

// EM Processes
#include "G4GammaConversion.hh"
#include "G4ComptonScattering.hh"
#include "G4PhotoElectricEffect.hh"
#include "G4eMultipleScattering.hh"
#include "G4eIonisation.hh"
#include "G4eBremsstrahlung.hh"
#include "G4eplusAnnihilation.hh"

PhyList::PhyList() {
	defaultCutValue = 0.5*mm;
}

PhyList::~PhyList() {}

void PhyList::ConstructParticle() {
	G4Gamma::GammaDefinition();
	G4Electron::ElectronDefinition();
	G4Positron::PositronDefinition();
}

void PhyList::SetCuts() {
	SetCutsWithDefault();
}

void PhyList::ConstructProcess() {
	AddTransportation();

	// Construct EM
	auto theParticleIterator = GetParticleIterator();
	theParticleIterator->reset();
	while( (*theParticleIterator)() ) {
		G4ParticleDefinition* particle = theParticleIterator->value();
		G4ProcessManager* pManager = particle->GetProcessManager();
		G4String particleName = particle->GetParticleName();

		if(particleName == "gamma") {
			pManager->AddDiscreteProcess(new G4GammaConversion);
			pManager->AddDiscreteProcess(new G4ComptonScattering);
			pManager->AddDiscreteProcess(new G4PhotoElectricEffect);
		} else if(particleName == "e-") {
			pManager->AddProcess(new G4eMultipleScattering,-1, 1, 1);
			pManager->AddProcess(new G4eIonisation,        -1, 2, 2);
			pManager->AddProcess(new G4eBremsstrahlung,    -1, 3, 3);
		} else if(particleName == "e+") {
			pManager->AddProcess(new G4eMultipleScattering,-1, 1, 1);
			pManager->AddProcess(new G4eIonisation,        -1, 2, 2);
			pManager->AddProcess(new G4eBremsstrahlung,    -1, 3, 3);
			pManager->AddProcess(new G4eplusAnnihilation,   0,-1, 4);
		}
	}
}