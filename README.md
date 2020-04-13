# G4-CT-Dose
This program calculate the dose inside a CT phantom for a set of beam positions and energies.

## scan.ini

## .img file format
* **int32, int32, int32**: voxel number on X/Y/Z (nx, ny & nz)
* **float32, float32, float32**: voxel size on X/Y/Z
* **int16[`nx*ny*nz`]**: HU number of voxels, in the order of X, Y, and then Z

## .dose/.err file format
* **int32, int32, int32**: voxel number on X/Y/Z (nx, ny & nz)
* **float32, float32, float32**: voxel size on X/Y/Z
* **int32, int32**: X/Z voxel index of beam center
* **float32**: particle energy in MeV
* **int32, int32**: Xmin, Xmax of the voxel index
* **int32, int32**: Zmin, Zmax of the voxel index
* **float32[`(Xmax-Xmin+1)*ny*(Zmax-Zmin+1)`]**: Dose/Error in voxels, in the order of X, Y, and then Z

## HU2mat.py
The coversion of CT number (HU) to densities and elemental weights is based on the paper of [Schneider _et al._, _Phys. Med. Biol._ 45 (2000) 459-478](https://doi.org/10.1088/0031-9155/45/2/314).  
### Densities
The densities are linearly interpolated between materials as described in the paper:

HU Range | Density Interpolation
  :---:  |----------------
-1000 - -98 | Between Air and Adipose Tissue 3 (Eq. 17)
-98 - 14 | Between Adipose Tissue 3 and Adrenal Gland (Eq. 21)
14 - 23 | Constant as 1.03 g/cm<sup>3</sup>
23 - 100 | Between Small Intenstine and Connective Tissue (Eq. 23)
\>100 | Between Yellow/Red Marrow and Cortical Bone (Eq. 19)

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