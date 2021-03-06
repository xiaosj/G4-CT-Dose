#include "ScanInfo.hh"
#include <string.h>
#include "stdio.h"
#include <iostream>
#include <iomanip>

#include "G4SystemOfUnits.hh"
#include "G4NistManager.hh"

#define NZONE  7

ScanInfo::ScanInfo() {
	if( (logfile = fopen("scan.log", "a+")) == NULL) {
		printf("Error: Fail to open logfie file scan.log\n");
		abort();
	}
	time_t this_t = time(NULL);
	struct tm tm = *localtime(&this_t);
	sprintf(logstr, "\nRun starts at %d-%02d-%02d %02d:%02d:%02d\n\n",
	        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			  tm.tm_hour, tm.tm_min, tm.tm_sec);
	writeLog();

	FILE* input;
	char line[MAX_INPUT_LINE_LENGTH];

	if( (input = fopen("scan.ini", "r")) == NULL) {
		sprintf(logstr, "Cannot open scan.ini\n");
		writeLog();
		exit(-1);
	} 
	sprintf(logstr, "Reading scan.ini ......\n");
	writeLog();

	fscanf(input, "%s %s %s", line, line, ct_filename);
	fscanf(input, "%s %s %s", line, line, mat_filename);
	fscanf(input, "%s %s %s", line, line, particle);
	
	// fscanf(input, "%[^\n]", line);
	fgets(line, sizeof(line), input);  // read the "\n" from the previous fscanf
	fgets(line, sizeof(line), input);
	n_fixed_esteps = 0;
	char* pch;
	pch = strtok(line, " ,");
	pch = strtok(NULL, " ,");  // skip the name
	pch = strtok(NULL, " ,");  // skip the "="
	while(pch != NULL) {
		fixed_esteps[n_fixed_esteps] = atof(pch);
		pch = strtok(NULL, " ,");
		n_fixed_esteps++;
		if(n_fixed_esteps == MAX_FIEXED_ESTEPS) {
			printf("Exceed the maximum fixed energy steps (%d)\n", MAX_FIEXED_ESTEPS);
			exit(-2);
		}
	}
	fscanf(input, "%s %s %f %f %d", line, line, &random_emin, & random_emax, &n_random_esteps);

	fscanf(input, "%s %s %d %d %d", line, line, &scan_x1, &scan_x2, &scan_dx);
	fscanf(input, "%s %s %d %d %d", line, line, &scan_z1, &scan_z2, &scan_dz);
	fscanf(input, "%s %s %lf %lf", line, line, &target_pct, &target_std);
	fscanf(input, "%s %s %d", line, line, &primary_batch);
	fscanf(input, "%s %s %ld", line, line, &maxPrimary);
	int tmp;
	fscanf(input, "%s %s %d", line, line, &tmp);
	if(tmp != 0)  output_full = true;
	else          output_full = false;
	fscanf(input, "%s %s %lf", line, line, &output_range_mm);
	fscanf(input, "%s %s %d", line, line, &verbose);

	fclose(input);

	readMaterial();
	readCT();
	init();
}

ScanInfo::~ScanInfo() {
	delete [] matID;
	delete [] dose;
	delete [] dose_err;
	delete [] s1;
	delete [] s2;
	delete [] de;
	delete [] HU2matID;
	time_t this_t = time(NULL);
	struct tm tm = *localtime(&this_t);
	sprintf(logstr, "\nRun finishes at %d-%02d-%02d %02d:%02d:%02d\n\n",
	        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			  tm.tm_hour, tm.tm_min, tm.tm_sec);
	writeLog();
	fclose(logfile);
}


void ScanInfo::writeLog() {
	printf("%s", logstr);
	fputs(logstr, logfile);
}


void ScanInfo::init() {
	dose     = new G4float[ctnx * ctny * ctnz];
	dose_err = new G4float[ctnx * ctny * ctnz];
	de       = new G4double[ctnx * ctny * ctnz];
	s1       = new G4double[ctnx * ctny * ctnz];
	s2       = new G4double[ctnx * ctny * ctnz];
	if (dose == 0 || dose_err == 0) {
		sprintf(logstr, "Initilization Error: Fail to create the dose matrix.\n");
		writeLog();
		abort();
	}

	for (int i = 0; i < ctnx*ctny*ctnz; i++) {
		de[i] = 0.0;
		s1[i] = 0.0;
		s2[i] = 0.0;
	}

	maxbatch = maxPrimary / primary_batch;
	if(maxbatch % 2 != 0)	maxbatch++;

	ctdx *= mm;		ctdy *= mm;		ctdz *= mm;
	voxelVolum = ctdx * ctdy * ctdz;
	ctdnx = (int)(output_range_mm / ctdx - 0.5) + 1;
	ctdnz = (int)(output_range_mm / ctdz - 0.5) + 1;
	
	G4int xmin, xmax, zmin, zmax;
	if(output_range_mm >= ctnx * ctdx * 0.5) {
		xmin = ctnx / 2 - 1;
		xmax = xmin;
	} else {
		xmin = ctdnx + 1;
		xmax = ctnx - 1 - xmin;
	}
	if(output_range_mm >= ctnz * ctdz * 0.5) {
		zmin = ctnz / 2 - 1;
		zmax = zmin;
	} else {
		zmin = ctdnz + 1;
		zmax = ctnz - 1 - zmin;
	}

	if(scan_x1 < xmin) {
		sprintf(logstr, "Warning: scan xmin %d is lower than the minimum, adjusted to %d\n", scan_x1, xmin);
		writeLog();
		scan_x1 = xmin;
		if(scan_x2 < scan_x1)  scan_x2 = scan_x1;
	}
	if(scan_x2 > xmax) {
		sprintf(logstr, "Warning: scan xmax %d is larger than the maximum, adjusted to %d\n", scan_x2, xmax);
		writeLog();
		scan_x2 = xmax;
		if(scan_x1 > scan_x2)  scan_x1 = scan_x2;
	}

	if(scan_z1 < zmin) {
		sprintf(logstr, "Warning: scan zmin %d is lower than the minimum, adjusted to %d\n", scan_z1, zmin);
		writeLog();
		scan_z1 = zmin;
		if(scan_z2 < scan_z1)  scan_z2 = scan_z1;
	}
	if(scan_z2 > zmax) {
		sprintf(logstr, "Warning: scan zmax %d is larger than the maximum, adjusted to %d\n", scan_z2, zmax);
		writeLog();
		scan_z2 = zmax;
		if(scan_z2 < scan_z1)  scan_z1 = scan_z2;
	}
	if(scan_z2 < scan_z1)  scan_z2 = scan_z1;

	sprintf(logstr, "Scan range is X(%d, %d, %d), Z(%d, %d, %d), Energy(%.2f, %.2f, %.2f)\n", scan_x1, scan_x2, scan_dx, scan_z1, scan_z2, scan_dz, scan_e1, scan_e2, scan_de);
	writeLog();

	sprintf(logstr, "Scan in %d Fixed and %d Random energy steps.\n", n_fixed_esteps, n_random_esteps);
	writeLog();

	if(output_full) {
		sprintf(logstr, "Will output dose arrays matching the whole CT view.\n");
		writeLog();
	} else {
		sprintf(logstr, "Will output dose array in dX = %d, dZ = %d.\n", ctdnx, ctdnz);
		writeLog();
	}

	sprintf(logstr, "\n\n");
	writeLog();
}


void ScanInfo::readMaterial() {
	FILE* matFile;
	G4int total_mat, nElements;
	G4double rho[CT_MATERIALS];
	G4double elements[CT_MATERIALS][CT_ELEMENTS];

	if((matFile = fopen(mat_filename, "r")) == NULL) {
		G4cout << "Error: Cannot open material file: " << mat_filename << G4endl;
		abort();
	}
	G4cout << "Reading " << mat_filename << " ......" << G4endl;
	
	fscanf(matFile, "%d  %d", &total_mat, &nElements);
	if(total_mat != CT_MATERIALS) {
		G4cout << "Error: Must use the definition of " << CT_MATERIALS << " mateirals." << G4endl;
		abort();
	}

	for(int i = 0; i < CT_MATERIALS; i++) {
		fscanf(matFile, "%d", &ct_range[i]);
		fscanf(matFile, "%lf", &rho[i]);
		for(int x = 0; x < CT_ELEMENTS; x++)
			fscanf(matFile, "%lf", &elements[i][x]);
	}
	fclose(matFile);

	// Create elements
	G4NistManager* man = G4NistManager::Instance();
	G4Element* el[CT_ELEMENTS];
	el[0] = man->FindOrBuildElement("H");
	el[1] = man->FindOrBuildElement("C");
	el[2] = man->FindOrBuildElement("N");
	el[3] = man->FindOrBuildElement("O");
	el[4] = man->FindOrBuildElement("Na");
	el[5] = man->FindOrBuildElement("Mg");
	el[6] = man->FindOrBuildElement("P");
	el[7] = man->FindOrBuildElement("S");
	el[8] = man->FindOrBuildElement("Cl");
	el[9] = man->FindOrBuildElement("Ar");
	el[10] = man->FindOrBuildElement("K");
	el[11] = man->FindOrBuildElement("Ca");

	// Create materials
	G4int nel;
	mat[0] = man->FindOrBuildMaterial("G4_AIR");
	for(int i = 0; i < CT_MATERIALS; i++) {
		nel = 0;
		for(int j = 0; j < CT_ELEMENTS; j++) {
			if(elements[i][j] > 0)	nel++;
		}
		char matName[10];
		sprintf(matName, "ctMat%d", i+1);
		mat[i+1] = new G4Material(matName, rho[i]*gram/cm3, nel);
		for(int j = 0; j < CT_ELEMENTS; j++) {
			if(elements[i][j] > 0)
				mat[i+1]->AddElement(el[j], elements[i][j]*0.01);
		}
	}
	for(int i = 0; i<= CT_MATERIALS; i++)
		ctMat.push_back(mat[i]);

	// Map HU to material index
	int HU_max = ct_range[CT_MATERIALS-1];
	HU2matID = new unsigned int[HU_max + 1000 + 1];
	for(int hu = -1000; hu <= HU_max; hu++) {
		if(hu <= ct_range[0])
			HU2matID[hu+1000] = 0;
		else
			for(int j = 1; j < CT_MATERIALS; j++)
				if(hu <= ct_range[j]) {
					HU2matID[hu+1000] = j+1;
					break;
				}
	}

	FILE* HU2mat_f = fopen("HU2mat.txt", "w");
	char tmp[200];
	fputs("# Materials for different HU values. The first row is for HU=-1000 and HU increases 1 per row.\n", HU2mat_f);
	fputs("#  HU Density(g/cm3)   H     C     N     O      Na     Mg     P      S      Cl     Ar      K     Ca\n", HU2mat_f);
	for(int hu = -1000; hu <= HU_max; hu++) {
		int id;
		if(hu <= -1000)
			id = 0;
		else if (hu >= ct_range[CT_MATERIALS-1])
			id = CT_MATERIALS;
		else 
			id = HU2matID[hu+1000];

		G4double density = mat[id]->GetDensity() / (gram / cm3);
		sprintf(tmp, "%5d  %10.5f ", hu, density);
		fputs(tmp, HU2mat_f);

		// Reduce id by 1 since mat[0] is air
		// mat[1] has the same elemental fraction of air
		if(id > 0)  id--;
		for(int j = 0; j < CT_ELEMENTS; j++) {
			sprintf(tmp, " %6.3f", elements[id][j]);
			fputs(tmp, HU2mat_f);
		}
		fputs("\n", HU2mat_f);

		// TODO output elemental fractions directly from G4Material
		// size_t nelement = mat[id]->GetNumberOfElements();
		// const G4double* fraction_vec = mat[id]->GetFractionVector();
		// for(size_t i = 0; i < nelement; i++) {
		// 	sprintf(tmp, " %6.3f", fraction_vec[i]*100);
		// 	fputs(tmp, HU2mat_f);
		// }
	}
	fclose(HU2mat_f);

	// Print all the materials defined
	// G4cout << G4endl << "The materials defined are : " << G4endl << G4endl;
	// G4cout << *(G4Material::GetMaterialTable()) << G4endl;
	// for(int i = 0; i<= CT_MATERIALS; i++) {
	// 	printf("mat[%d]: ", i);		
	// 	G4cout << mat[i] << G4endl;
	// }

	sprintf(logstr, "ctMat_Idx   min_CT#\n");
	writeLog();
	for(int i =0; i < CT_MATERIALS; i++) {
		sprintf(logstr,  "   %2d         %d\n", i+1, ct_range[i]);
		writeLog();
	}
	sprintf(logstr, "\n");
	writeLog();
}


void ScanInfo::readCT() {
	FILE* ctFile;
	if((ctFile = fopen(ct_filename, "rb")) == NULL) {
		G4cout << "Error: Cannot open CT file: " << ct_filename << G4endl;
		abort();
	}
	G4cout << "Reading CT file: " << ct_filename << " ......";

	// Head of img file
	fread(&ctnx, sizeof(int), 1, ctFile);
	fread(&ctny, sizeof(int), 1, ctFile);
	fread(&ctnz, sizeof(int), 1, ctFile);
	fread(&ctdx, sizeof(float), 1, ctFile);
	fread(&ctdy, sizeof(float), 1, ctFile);
	fread(&ctdz, sizeof(float), 1, ctFile);

	// Map CT number to material
	matID = new size_t[ctnx * ctny * ctnz];
	short hu;
	nonAirV = 0;
	for(int i = 0; i < ctnx * ctny * ctnz; i++) {
		fread(&hu, 2, 1, ctFile);
		if(hu <= -1000)
			matID[i] = 0;
		else if (hu >= ct_range[CT_MATERIALS-1])
			matID[i] = CT_MATERIALS;
		else 
			matID[i] = HU2matID[hu+1000];
		if(matID[i] > 0)	nonAirV++;
	}
	G4cout << "  " << nonAirV << " non-Air voxels." << G4endl;
	fclose(ctFile);
	printf("  - (%d * %d * %d) voxels at (%.2f * %.2f * %.2f) mm\n", ctnx, ctny, ctnz, ctdx, ctdy, ctdz);

	// FILE* fw;
	// fw = fopen("matID.img", "wb");
	// fwrite(&ctnx, sizeof(int), 1, fw);
	// fwrite(&ctny, sizeof(int), 1, fw);
	// fwrite(&ctnz, sizeof(int), 1, fw);
	// fwrite(&ctdx, sizeof(float), 1, fw);
	// fwrite(&ctdy, sizeof(float), 1, fw);
	// fwrite(&ctdz, sizeof(float), 1, fw);
	// fwrite(matID, sizeof(size_t), ctnx * ctny * ctnz, fw);
	// fclose(fw);
}

void ScanInfo::calDose() {
	if(nbatch % 2 == 1) {
		// printf("Writing to s1\n");
		for(int i = 0; i < ctnx * ctny * ctnz; i++) {
			s1[i] += de[i];
			de[i] = 0.0;
		}
	}
	else {
		// printf("Writing to s2\n");
		for(int i = 0; i < ctnx * ctny * ctnz; i++) {
			s2[i] += de[i];
			de[i] = 0.0;
		}
	}
}


// Write file head
void ScanInfo::writeHead(FILE *fw) {
	fwrite(&ctnx, sizeof(G4int), 1, fw);
	fwrite(&ctny, sizeof(G4int), 1, fw);
	fwrite(&ctnz, sizeof(G4int), 1, fw);
	fwrite(&ctdx, sizeof(float), 1, fw);
	fwrite(&ctdy, sizeof(float), 1, fw);
	fwrite(&ctdz, sizeof(float), 1, fw);
	fwrite(&cx, sizeof(G4int), 1, fw);
	fwrite(&cz, sizeof(G4int), 1, fw);
	float energy_MeV = energy * MeV;
	fwrite(&energy_MeV, sizeof(float), 1, fw);

	// Range of dose voxels
	G4int x0, x1, z0, z1;
	if(output_full) {
		x0 = 0;		x1 = ctnx - 1;
		z0 = 0;		z1 = ctnz - 1;
	} else {
		x0 = cx - ctdnx;	x1 = cx + ctdnx;
		z0 = cz - ctdnz;	z1 = cz + ctdnz;
	}
	fwrite(&x0, sizeof(G4int), 1, fw);
	fwrite(&x1, sizeof(G4int), 1, fw);
	fwrite(&z0, sizeof(G4int), 1, fw);
	fwrite(&z1, sizeof(G4int), 1, fw);
}


G4double ScanInfo::get_unit_factor() {
	// nGy/primary
	return 1.0 / (gray * 1e-9 * voxelVolum * G4float(nbatch * primary_batch));
}

// Write data to file.
// cx, cz: index of beam center, used when Output_full is not True
void ScanInfo::writeDose() {
	FILE* fw1;
	FILE* fw2;
	char file1[MAX_FILENAME_LENGTH];
	char file2[MAX_FILENAME_LENGTH];
	sprintf(file1, "%s.dose", filePre);
	sprintf(file2, "%s.err", filePre);
	if( (fw1 = fopen(file1, "wb")) == NULL) {
		sprintf(logstr, "Error: Fail to write dose file: %s\n", file1);
		writeLog();
		abort();
	}
	if( (fw2 = fopen(file2, "wb")) == NULL) {
		sprintf(logstr, "Error: Fail to write dose error file: %s\n", file2);
		writeLog();
		abort();
	}

	writeHead(fw1);
	writeHead(fw2);

	G4double unit_factor = get_unit_factor();
	for(int i = 0; i < ctnx * ctny * ctnz; i++) {
			dose[i] *= unit_factor;
	}

	if(output_full) {  // Output full array
		fwrite(dose, sizeof(G4float), ctnx * ctny * ctnz, fw1);
		fclose(fw1);
		fwrite(dose_err, sizeof(G4float), ctnx * ctny * ctnz, fw2);
		fclose(fw2);
	} 
	else {  // Output the array part higher than output_cut only
		G4double maxDose = 0.0;
		for(int i = 0; i < ctnx * ctny * ctnz; i++) {
			if(dose[i] > maxDose)	maxDose = dose[i];
		}
		sprintf(logstr, "Output dose range X = [%d, %d], Z = [%d, %d]\n",
		        cx - ctdnx, cx + ctdnx, cz - ctdnz, cz + ctdnz);
		writeLog();
		int ctnxy = ctnx * ctny;
		int lx = 2 * ctdnx + 1;
		for(int iz = cz - ctdnz; iz <= cz + ctdnz; iz++) {
			for(int iy = 0; iy < ctny; iy++) {
				G4int idx = cx - ctdnx + iy * ctnx + iz * ctnxy;
				fwrite(&dose[idx], sizeof(G4float), lx, fw1);
				fwrite(&dose_err[idx], sizeof(G4float), lx, fw2);
			}
		}
		fclose(fw1);
		fclose(fw2);
	}
}

bool ScanInfo::reachTarget() {
	G4double maxDose = 0.0;
	G4double unit_factor = get_unit_factor();
	for(int i = 0; i < ctnx * ctny * ctnz; i++) {
		dose[i] = (s1[i] + s2[i]) / ctMat[matID[i]]->GetDensity();
		if(dose[i] > 0.0)
			dose_err[i] = abs(s1[i] - s2[i]) / (s1[i] + s2[i]);
		else
			dose_err[i] = 0.0;
		if(dose[i] > maxDose)	maxDose = dose[i];
	}

	G4float doseCut = maxDose * target_pct;
	printf("    max dose = %.2e, cut = %.2e nGy/primary\n", maxDose*unit_factor, doseCut*unit_factor);
	
	G4int nonZero = 0;
	G4int nCut = 0;
	G4int nTarget = 0;
	for(int i = 0; i < ctnx * ctny * ctnz; i++) {
		if(dose[i] > 0.0)		nonZero++;		
		if(dose[i] > doseCut) {
			nCut++;
			if(dose_err[i] <= target_std)
				nTarget++;
		}
	}
	sprintf(logstr, "    %d/%d (%.1f%%) > %.1f%% max dose.\n", nCut, nonZero,
	       100.0*(float)nCut/(float)nonZero, target_pct * 100.0);
	writeLog();
	sprintf(logstr, "    %d/%d (%.1f%%) < %.1f%% err.\n", nTarget, nCut,
	       100.0*(float)nTarget/(float)nCut, target_std * 100.0);
	writeLog();

	if(verbose > 0) {
		G4float stat_ratio[NZONE] = {0.5, 0.2, 0.1, 0.05, 0.02, 0.01, 0.005};
		G4float stat_dose[NZONE];
		G4int stat_cnt[NZONE];
		G4int stat_x0[NZONE], stat_x1[NZONE];
		G4int stat_z0[NZONE], stat_z1[NZONE];
		G4float stat_sum[NZONE], stat_sum2[NZONE];
		for(int i=0; i < NZONE; i++) {
			stat_dose[i] = stat_ratio[i] * maxDose;
			stat_cnt[i] = 0;
			stat_x0[i] = ctnx;		stat_x1[i] = 0;
			stat_z0[i] = ctnz;		stat_z1[i] = 0;
			stat_sum[i] = 0.0;		stat_sum2[i] = 0.0;
		}

		G4int idx = 0;
		for(int iz = 0; iz < ctnz; iz++) {
			for(int iy = 0; iy < ctny; iy++) {
				for(int ix = 0; ix < ctnx; ix++) {
					for(int j = 0; j < NZONE; j++) {
						if(dose[idx] > stat_dose[j]) {
							stat_cnt[j]++;
							stat_sum[j] += dose_err[idx];
							stat_sum2[j] += dose_err[idx] * dose_err[idx];
							if(stat_x0[j] > ix)   stat_x0[j] = ix;
							if(stat_x1[j] < ix)   stat_x1[j] = ix;
							if(stat_z0[j] > iz)   stat_z0[j] = iz;
							if(stat_z1[j] < iz)   stat_z1[j] = iz;
							break;
						}
					}
					idx++;
				}
			}
		}
		sprintf(logstr, "              Count (%%)       Error%% (Mean/Std)    Range (X & Z)\n");
		writeLog();
		for(int j = 0; j < NZONE; j++) {
			G4int d0;
			if(j == 0) d0 = 100;
			else d0 = stat_ratio[j-1] * 100;
			G4int d1 = stat_ratio[j] * 100;
			if(d1 > 0) {
				sprintf(logstr, "%3d%% - %3d%%  %7d (%5.2f%%)     %4.1f+/-%4.1f     X=%.1fmm, Z=%.1fmm\n", 
					d0, d1, stat_cnt[j], stat_cnt[j] * 100.0 / nonZero,
					stat_sum[j] / stat_cnt[j] * 100.0, sqrt(stat_sum2[j] / stat_cnt[j] - pow(stat_sum[j]/stat_cnt[j], 2)) * 100.0,
					(stat_x1[j] - stat_x0[j] + 1) * ctdx, (stat_z1[j] - stat_z0[j] + 1) * ctdz);
			} else {
				sprintf(logstr, "%3.1f%% - %3.1f%%  %7d (%5.2f%%)     %4.1f+/-%4.1f     X=%.1fmm. Z=%.1fmm\n", 
					stat_ratio[j-1] * 100, stat_ratio[j] * 100, stat_cnt[j], stat_cnt[j] * 100.0 / nonZero,
					stat_sum[j] / stat_cnt[j] * 100.0, sqrt(stat_sum2[j] / stat_cnt[j] - pow(stat_sum[j]/stat_cnt[j], 2)) * 100.0,
					(stat_x1[j] - stat_x0[j] + 1) * ctdx, (stat_z1[j] - stat_z0[j] + 1) * ctdz);
			}
			writeLog();
		}
		sprintf(logstr, "\n");
		writeLog();
	}

	if(nbatch >= maxbatch) {
		printf("Stop simulation. Max primary particle number reached.\n");
		return true;
	} else
		return (nTarget == nCut);
}


void ScanInfo::reset(int cx_in, int cz_in, float energy_in) {
	for (int i = 0; i < ctnx*ctny*ctnz; i++) {
		de[i]    = 0.0;
		s1[i]    = 0.0;
		s2[i]    = 0.0;
	}
	cx = cx_in;
	cz = cz_in;
	energy = energy_in;
}
