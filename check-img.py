# Check the parameters of .img and .dose file
#   Usage: python check-img.py [filename]

import numpy as np
import sys
import struct

class img_data:
  ''' Voxel array in the order of [ix, iy, iz]
  '''
  def __init__(self, img_file):
    with open(img_file, 'rb') as f:
      self.nx, self.ny, self.nz = struct.unpack('iii', f.read(12))
      self.dx, self.dy, self.dz = struct.unpack('fff', f.read(12))
      self.voxel = np.fromfile(f, dtype=np.int16, count=self.nx*self.ny*self.nz).reshape(self.nz, self.ny, self.nx)
      f.close()

  def print_info(self):
    print('Image voxel shape: {}'.format(self.voxel.shape))
    print('Image voxel size:  ({:.3f}, {:.3f}, {:.3f}) mm'.format(self.dx, self.dy, self.dz))

class dose_data:
  def __init__(self, dose_file):
    with open(dose_file, 'rb') as f:
      self.nx, self.ny, self.nz = struct.unpack('iii', f.read(12))
      self.dx, self.dy, self.dz = struct.unpack('fff', f.read(12))
      self.dose_x, self.dose_z, self.dose_e = struct.unpack('iif', f.read(12))
      self.dose_x0, self.dose_x1 = struct.unpack('ii', f.read(8))
      self.dose_z0, self.dose_z1 = struct.unpack('ii', f.read(8))
      self.dose_nx = self.dose_x1 - self.dose_x0 + 1
      self.dose_nz = self.dose_z1 - self.dose_z0 + 1
      self.dose = np.fromfile(f, dtype=np.float32,
          count=self.dose_nx * self.ny * self.dose_nz).reshape(self.dose_nz, self.ny, self.dose_nx)
      f.close()

  def output_turple(self):
    '''Output the turple to be saved for later'''
    return (self.dose_x, self.dose_z, self.dose_e, self.dose_nx, self.ny, self.dose_nz)

  def print_info(self):
    print('Dose voxel shape: {}'.format(self.dose.shape))
    print('Dose voxel size:  ({:.3f}, {:.3f}, {:.3f}) mm'.format(self.dx, self.dy, self.dz))
    print('Dose center:      ({}, {})'.format(self.dose_x, self.dose_z))
    print('Simulation energy: {:.3f} MeV'.format(self.dose_e))


if __name__ == '__main__':
  if len(sys.argv) != 2:
    print('Wrong input:')
    print('  python check-img.py [filename]')
    exit(-1)
  
  filename = sys.argv[1]
  if filename[-3:] == 'img':
    img = img_data(filename)
    img.print_info()
  elif filename[-4:] == 'dose':
    dose = dose_data(filename)
    dose.print_info()