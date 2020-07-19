#ifndef SCAN_INFO_HH
#define SCAN_INFO_HH 1

#include "ScanGlobals.hh"
#include "G4Material.hh"
#include "G4VPhysicalVolume.hh"

# define MAX_LOG_LENGTH   100

class ScanInfo {
public:
	ScanInfo();
	~ScanInfo();

	G4int    ctnx, ctny, ctnz;
	G4int    ctdnx, ctdnz;
	float    ctdx, ctdy, ctdz;
	G4double scan_e1, scan_e2, scan_de;
	G4int    scan_x1, scan_x2, scan_dx;
	G4int    scan_z1, scan_z2, scan_dz;
	G4double target_pct, target_std;
	G4int    primary_batch;
	G4float* dose;
	G4float* dose_err;
	G4double* s1;
	G4double* s2;
	G4double* de;
	G4int     nbatch;
	char      particle[16];
	G4int     maxbatch;
	int       cx, cz;
	double    energy;
	int       n_fixed_esteps;
	float     fixed_esteps[MAX_FIEXED_ESTEPS];
	float     random_emin, random_emax;
	int       n_random_esteps;

	std::vector<G4Material*> ctMat;  // materials
	size_t* matID;  // material ID of each voxle
	G4VPhysicalVolume* ctPV;
	G4Material* mat[CT_MATERIALS+1];

	char filePre[MAX_FILENAME_LENGTH-10];
	char logstr[MAX_LOG_LENGTH];
	void writeLog();
	void calDose();
	void writeDose();
	bool reachTarget();
	void reset(int cx_in, int cz_in, float energy_in);

private:
	char  ct_filename[MAX_FILENAME_LENGTH];
	char mat_filename[MAX_FILENAME_LENGTH];
	G4double get_unit_factor();
	G4int ct_range[CT_MATERIALS];
	G4double voxelVolum;
	G4int nonAirV;
	long maxPrimary;
	bool output_full;
	int verbose;
	G4double output_range_mm;
	void init();
	void readMaterial();
	void readCT();
	void writeHead(FILE* fw);
	FILE* logfile;
	unsigned int* HU2matID;
};

#endif