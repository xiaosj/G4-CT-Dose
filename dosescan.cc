#include "globals.hh"
// #ifdef G4MULTITHREADED
// #include "G4MTRunManager.hh"
// #else
#include "G4RunManager.hh"
// #endif
#include "G4UImanager.hh"
#include "G4UIterminal.hh"
#include "G4Timer.hh"
#include "G4VModularPhysicsList.hh"
#include "Shielding.hh"

#include "ScanGlobals.hh"
#include "ScanInfo.hh"
#include "VoxelConstruction.hh"
#include "PhyList.hh"
#include "PrimaryGen.hh"
#include "VoxelStep.hh"

#include "stdio.h"
#include <iostream>

int main(int argc,char** argv)
{
	G4Timer gTimer;
	gTimer.Start();
	time_t start_t = time(NULL);

	// Initialize
	ScanInfo* scan = new ScanInfo();

	// #ifdef G4MULTITHREADED
	// 	G4MTRunManager* runManager = new G4MTRunManager;
	// 	G4cout << "Number of cores: " << G4Threading::G4GetNumberOfCores() << G4endl;
	// 	runManager->SetNumberOfThreads(G4Threading::G4GetNumberOfCores());
	// #else
		sprintf(scan->logstr, "Running on single CPU.\n");
		scan->writeLog();
		G4RunManager* runManager = new G4RunManager;
	// #endif

	VoxelConstruction* ctVoxel = new VoxelConstruction(scan);
	runManager->SetUserInitialization(ctVoxel);

	// G4VModularPhysicsList* phyList = new Shielding;
	// phyList->SetVerboseLevel(0);
	// runManager->SetUserInitialization(phyList);
	runManager->SetUserInitialization(new PhyList);

	runManager->SetUserAction(new PrimaryGen);

	VoxelStep* step = new VoxelStep(scan);
	G4UserSteppingAction* g4step = step;
	runManager->SetUserAction(g4step);

	runManager->Initialize();

	G4UImanager* UI = G4UImanager::GetUIpointer();
	UI->ApplyCommand("/run/verbose 0");
	UI->ApplyCommand("/event/verbose 0");
	UI->ApplyCommand("/tracking/verbose 0");

	// input parameters
	if(argc > 1)  {
		for(int i = 0; i< argc; i++) {
			sprintf(scan->logstr, "arg[%d] = %s\n", i, argv[i]);
			scan->writeLog();
		}
		if(G4String(argv[1]) == "-new")   {
			time_t seconds = time(NULL);
			CLHEP::HepRandom::setTheSeed(seconds);
			sprintf(scan->logstr, "New random seed are generated from timpstamp.\n");
			scan->writeLog();
		}
		else {
			sprintf(scan->logstr, "Default random seed are applied.\n");
			scan->writeLog();
		}
	}
	sprintf(scan->logstr, "Random seed = %ld\n", CLHEP::HepRandom::getTheSeed());
	scan->writeLog();

	char cmd[64];
	sprintf(cmd, "/gun/particle %s", scan->particle);
	UI->ApplyCommand(cmd);
	sprintf(cmd, "/gun/direction 0 1 0");
	UI->ApplyCommand(cmd);

	int esteps = (scan->scan_e2 - scan->scan_e1) / scan->scan_de + 1;
	int zsteps = (scan->scan_z2 - scan->scan_z1) / scan->scan_dz + 1;
	int xsteps = (scan->scan_x2 - scan->scan_x1) / scan->scan_dx + 1;
	int nsteps = esteps * zsteps * xsteps;
	int istep = 0;

	for(double energy = scan->scan_e1; energy <= scan->scan_e2; energy += scan->scan_de) {
		sprintf(cmd, "/gun/energy %.2f MeV", energy);
		UI->ApplyCommand(cmd);

		for(int iz = scan->scan_z1; iz <= scan->scan_z2; iz += scan->scan_dz) {
			for(int ix = scan->scan_x1; ix <= scan->scan_x2; ix += scan->scan_dx) {
				G4Timer sTimer;
				sTimer.Start();

				double px, py, pz;
				px = ((G4double)ix - scan->ctnx/2. + 0.5) * scan->ctdx;
				py = -scan->ctny * scan->ctdy * 0.5 - 10;
				pz = ((G4double)iz - scan->ctnz/2. + 0.5) * scan->ctdz;
				sprintf(cmd, "/gun/position %.2f %.2f %.2f mm", px, py, pz);
				UI->ApplyCommand(cmd);

				istep++;
				sprintf(scan->logstr, "\nRun %d/%d: %.2f MeV beams at (%d,%d)->(%.2f,%.2f)\n",
					     istep, nsteps, energy/MeV, ix, iz, px, pz);
				scan->writeLog();

				sprintf(scan->filePre, "%d-%d", ix, iz);
				scan->nbatch = 0;
				scan->reset();
				do {
					for(int in = 0; in < 2; in++) {
						G4Timer bTimer;
						bTimer.Start();
						scan->nbatch++;
						sprintf(scan->logstr, "  Run %d/%d Batch %d/%d: %d primaries",
						        istep, nsteps, scan->nbatch, scan->maxbatch, scan->primary_batch);
						scan->writeLog();

						sprintf(cmd, "/run/beamOn %d", scan->primary_batch);
						UI->ApplyCommand(cmd);
						scan->calDose();
						bTimer.Stop();

						double complete_ratio = (istep - 1. + float(scan->nbatch) / scan->maxbatch) / nsteps;
						double total_time = difftime(time(NULL), start_t);
						sprintf(scan->logstr, ", done in %.1f s (%.1f%%, %.0f s elapsed, %.0f s remaining)\n",
							     bTimer.GetRealElapsed(), complete_ratio*100., total_time,
							     total_time *(1. - complete_ratio) / complete_ratio);
						scan->writeLog();
					}
				}while( !(scan->reachTarget()) );
				scan->writeDose(ix, iz, energy);

				sTimer.Stop();
				sprintf(scan->logstr, "  The scan finishes in %.0f s.\n", sTimer.GetRealElapsed());
				scan->writeLog();
			}
		}
	}
	gTimer.Stop();
	sprintf(scan->logstr,  "\nAll scans finish in %.0f s.\n", gTimer.GetRealElapsed());
	scan->writeLog();

	time_t end_t = time(NULL);
	struct tm tm = *localtime(&end_t);
	sprintf(scan->logstr, "\nRun finishes at %d-%02d-%02d %02d:%02d:%02d\n",
	        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			  tm.tm_hour, tm.tm_min, tm.tm_sec);
	scan->writeLog();
	
	delete runManager;
}
