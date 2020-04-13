# G4-CT-Dose
This program calculate the dose inside a CT phantom for a set of beam positions and energies.

## scan.ini
_Note: No `[]` in the real input file_
```
CT_file = [.img file]
material = [material file, e.g. mat_99.txt]
Beam_Type = [gamma, proton, electron, etc.]
Fixed_Energy_MeV = [fixed energy steps in MeV]
Random_Energy_MeV(Min,Max,N) = [1.0  1.0  0]
Beam_X = [minX maxX stepX]
Beam_Z = [minZ maxZ stepZ]
Error_Target = [dose_pct sim_err]  # the uncertainty of all doses larger than 'dose_pct' must be less than 'sim_err' 
Primary_per_Batch = [# of particles per batch]
Max_Primary = [# of max total particles]
Output_Full = [0/1]  # 1: output full voxels, 0: only save data in the output rante (the next)
Output_Range_mm = [output +/- ## mm from the beam ceter]
Verbose = [0/1/2, verbose level]
```

## .img file format
* **int32, int32, int32**: voxel number on X/Y/Z (nx, ny & nz)
* **float32, float32, float32**: voxel size on X/Y/Z
* **int16[`nx*ny*nz`]**: HU number of voxels, in the order of X, Y, and then Z

`class img_data` in `check-img.py` demonstrates how to read the `.img` file.

## .dose/.err file format
* **int32, int32, int32**: voxel number on X/Y/Z (nx, ny & nz)
* **float32, float32, float32**: voxel size on X/Y/Z
* **int32, int32**: X/Z voxel index of beam center
* **float32**: particle energy in MeV
* **int32, int32**: Xmin, Xmax of the voxel index
* **int32, int32**: Zmin, Zmax of the voxel index
* **float32[`(Xmax-Xmin+1)*ny*(Zmax-Zmin+1)`]**: Dose/Error in voxels, in the order of X, Y, and then Z

`class dose_data` in `check-img.py` demonstrates how to read the `.dose` file.

## Material Definitions
`HU2mat.py` defines the coversion of CT number (HU) to the densities and elemental weights of materials, based on the paper of [Schneider _et al._, _Phys. Med. Biol._ 45 (2000) 459-478](https://doi.org/10.1088/0031-9155/45/2/314).  
### Densities
The densities are linearly interpolated between materials as described in the paper:

HU Range | Density Interpolation
  :---:  |----------------
-1000 - -98 | Between Air and Adipose Tissue 3 (Eq. 17)
-98 - 14 | Between Adipose Tissue 3 and Adrenal Gland (Eq. 21)
14 - 23 | Constant as 1.03 g/cm<sup>3</sup>
23 - 100 | Between Small Intenstine and Connective Tissue (Eq. 23)
\>100 | Between Yellow/Red Marrow and Cortical Bone (Eq. 19)

_Note: To minimize the fluctration of CT scans on air, the HUs in the range from -1000 to the first HU in the material definition file use the density of air (i.e. no interpolation)._

### Elemental Weights
However, to have resolutions finer than the 24-bin setting in the paper, some deviations are introduced:

HU Range | In Paper | In Code
  :---:  |----------|--------
<-950 | Air | Air
-950 - -120 | Lung | Mean from interpolation
-120 - 18 | Eq. 22 | Mean from interpolation
18 - 80 | Mean from Eq. 24 | Mean from interpolation
20 - 120 | Connective Tissue | Mean from interpolation
\>120 | Eq. 20 | Mean from interpolation

* The paper used the interpolation value at the center of each bin except the bin at 18-80 and those explicitly listed above
* The code interpolate the elemental weights according to Eq. 18.
* The code uses the mean of the interpolated values of all HUs in a bin.

An ouput with 99 bins is given.