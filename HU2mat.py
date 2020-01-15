# Convert HU numbers to material density and composition
# baed on Schneider et al, Phys. Med. Biol. 45 (2000) 459-478

import numpy as np
from scipy.interpolate import interp1d

nelement = 12
mat = {#                  [  H    C     N    O     Na    Mg    P     S     Cl    Ar     K    Ca]
    'air': {
        'HU':  -950,      # Considered fluctuations
        'rho': 1.21e-3,   # Value in Schneider's paper. At 20 degC and 101.325 kPa, the density of dry air is 1.2041e-3.
        'elwt': np.array([ 0.0,  0.0, 75.5, 23.2,  0.0,  0.0,  0.0,  0.0,  0.0,  1.3,  0.0,  0.0])
    }, 
    'lung': {
        'HU':  -741,
        'rho': 0.26,
        'elwt': np.array([10.3, 10.5,  3.1, 74.9,  0.2,  0.0,  0.2,  0.3,  0.3,  0.0,  0.2,  0.0])
    },
    'at3': {  # Adipose Tissue 3
        'HU':  -98,
        'rho': 0.93,
        'elwt': np.array([11.6, 68.1,  0.2, 19.8,  0.1,  0.0,  0.0,  0.1,  0.1,  0.0,  0.0,  0.0])
    },
    'ma': {   # Yellow/Red Marrow (1:1)
        'HU': -22,
        'rho': 1.00,
        'elwt': np.array([11.0, 52.9,  2.1, 33.5,  0.0,  0.0,  0.1,  0.1,  0.2,  0.0,  0.1,  0.0])
    },
    'ag': {   # Adrenal Gland
        'HU': 14,
        'rho': 1.03, 
        'elwt': np.array([10.6, 28.4,  2.6, 57.8,  0.0,  0.0,  0.1,  0.1,  0.2,  0.0,  0.1,  0.0])
    },
    'si': {   # Small Intenstine (Wall)
        'HU': 23,
        'rho': 1.03,
        'elwt': np.array([10.6, 11.5,  2.2, 75.1,  0.1,  0.0,  0.1,  0.1,  0.2,  0.0,  0.1,  0.0])
    },
    'ct': {   # Connective Tissue
        'HU': 100,
        'rho': 1.12,
        'elwt': np.array([ 9.4, 20.7,  6.2, 62.2,  0.6,  0.0,  0.0,  0.6,  0.3,  0.0,  0.0, 0.0])
    },
    'cb': {   # Cortical Bone
        'HU': 1524,
        'rho': 1.92,
        'elwt': np.array([ 3.4, 15.5,  4.2, 43.5,  0.1,  0.2, 10.3,  0.3,  0.0,  0.0,  0.0, 22.5])
    }
}

def HU2rho(HU):
    ''' Convert HU to density, input can be a scalar or an array '''
    HU_x = np.array([   -1000,  -98,   14,   23,  100,    101, 1524])
    rho_y = np.array([1.21e-3, 0.93, 1.03, 1.03, 1.12, 1.0768, 1.92])
    rho = interp1d(HU_x, rho_y, kind='linear', bounds_error=False, fill_value='extrapolate')(HU)
    return rho

def HU2elwt(HU):
    if HU <= -950:
        elwt = mat['air']['elwt']
    elif HU <= mat['lung']['HU']:
        elwt = elwt_interpolate(mat['air'], mat['lung'], HU)
    elif HU <= mat['at3']['HU']:
        elwt = elwt_interpolate(mat['lung'], mat['at3'], HU)
    elif HU <= mat['ag']['HU']:
        elwt = elwt_interpolate(mat['at3'], mat['ag'], HU)
    elif HU <= mat['si']['HU']:
        elwt = elwt_interpolate(mat['ag'], mat['si'], HU)
    elif HU <= mat['ct']['HU']:
        elwt = elwt_interpolate(mat['si'], mat['ct'], HU)
    elif HU <= mat['cb']['HU']:
        elwt = elwt_interpolate(mat['ma'], mat['cb'], HU)
    else:
        elwt = mat['cb']['elwt']
    return elwt

def HU2elwt_Schneider(HU):
    ''' Convert HU to elemental weights, HU must be a scalar '''
    if HU <= -950:
        comp = mat['air']['elwt']
    elif HU <= -120:
        comp = mat['lung']['elwt']
    elif HU <= 14:
        comp = 0.93 * (14 - HU) / (114 + 0.1 * HU) * (mat['at3']['elwt'] - mat['ag']['elwt']) + mat['ag']['elwt']
    elif HU <= 23:
        comp = elwt_interpolate(mat['ag'], mat['si'], HU)
    elif HU <= 100:
        comp = 1.03 * (100 - HU) / (77 + 0.09 * HU) * (mat['si']['elwt'] - mat['ct']['elwt']) + mat['ct']['elwt']
    elif HU < 1524:
        comp = (1524 - HU) / (1566 + 0.92 * HU) * (mat['ma']['elwt'] - mat['cb']['elwt']) + mat['cb']['elwt']
    else:
        comp = mat['cb']['elwt']
    return comp

def elwt_interpolate(mat1, mat2, HU):
    ''' Interpolate elemental weights between two materials, HU must be a scalar '''
    elwt1 = mat1['elwt']
    elwt2 = mat2['elwt']
    r1 = mat1['rho']
    r2 = mat2['rho']
    H1 = mat1['HU']
    H2 = mat2['HU']
    elwt = r1 * (H2 - HU) / ((r1 * H2 - r2 * H1) + (r2 - r1) * HU) * (elwt1 - elwt2) + elwt2
    return elwt


HU_bin = np.concatenate((np.array([-1000, -950, -120, -83, -53, -23, 7, 18, 80, 120]), np.arange(200, 1700, 100)))

# Setting 1
HU_bin = np.concatenate((HU_bin, np.linspace(-950, -741, 5, dtype=np.int)[1:]))
HU_bin = np.concatenate((HU_bin, np.linspace(-741, -120, 13, dtype=np.int)[1:-1]))
HU_bin = np.concatenate((HU_bin, np.array([-101, -73, -63, -43, -33, -13, -3, 100, 140, 160, 180])))
HU_bin = np.concatenate((HU_bin, np.linspace(18, 80, 7, dtype=np.int)[1:-1]))
HU_bin = np.concatenate((HU_bin, np.arange(200, 1600, 25)))

# Setting 2 ...

# Start processing
HU_bin = np.sort(np.unique(HU_bin))
HUs = np.arange(HU_bin[0], HU_bin[-1])
nbin = len(HU_bin) - 1

elwts = np.zeros((len(HUs), nelement))
for i in range(len(HUs)):
    elwts[i] = HU2elwt(HUs[i])

idx_bin = HU_bin - HU_bin[0]
elwt_bin = np.zeros((nbin, nelement))
for i in range(nbin):
    elwt_bin[i] = np.mean(elwts[idx_bin[i]:idx_bin[i+1]], axis=0)

bin_low = HU_bin[:-1]
bin_high = HU_bin[1:]
bin_mean = (bin_low + bin_high) / 2.
rho_HU = HU2rho(bin_mean)

print('{:d} HU bins'.format(nbin))
for i in range(nbin):
    print('{:>5d} - {:>4d} [{:>4d}]: {:<7.4g}'.format(bin_low[i], bin_high[i], bin_high[i]-bin_low[i], rho_HU[i]), end=' ')
    print(('{:>6.2f}' * nelement).format(*elwt_bin[i]))

# Write material file
with open('mat_{:d}.txt'.format(nbin), 'w') as f:
    f.write('{:d}\n'.format(nbin))
    for i in range(nbin):
        f.write('  {:>5d}  {:<7.4f} '.format(bin_high[i], rho_HU[i]))
        f.write(('{:>6.2f}' * nelement).format(*elwt_bin[i]))
        f.write('\n')
    f.write('#   HU     rho      H     C     N     O     Na    Mg    P     S     Cl    Ar    K     Ca\n')
    f.write('#                   1     2     3     4     5     6     7     8     9     10    11    12\n\n')
    f.write('# HU to elemental weights from Schneider et al, Phys. Med. Biol. 45 (2000) 459-478\n')
    f.close()