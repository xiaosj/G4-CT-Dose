import os
import time
import subprocess
import shutil

global img_file, mat_file


def get_input(line):
    loc = line.find('=')
    return line[loc+1:].strip()


def read_limits():
    try:
        f = open('bsub_limits.inp', 'r')
    except:
        print('Cannot open bsub_limits.inp.\nTrying to create a defalut file.')
        try:
            f = open('bsub_limits.inp', 'w')
            f.write('max_jobs = 20\n')
            f.write('max_wall_time = 24:00\n')
            f.close()
            return 20, '24:00'
        except:
            raise Exception('Fail to create the default bsub_limits.inp')

    max_jobs = int(get_input(f.readline()))

    value = get_input(f.readline()).split(':')
    if len(value) == 1:
        max_wall_time = int(value[0]) * 3600
    elif len(value) == 2:
        max_wall_time = int(value[0]) * 3600 + int(value[1]) * 60
    else:
        raise Exception('Wrong wall time input. Must be HH:mm or HH.')
    
    spm = int(get_input(f.readline()))

    value = get_input(f.readline()).split(':')
    if len(value) == 1:
        tpj = int(value[0]) * 3600
    elif len(value) == 2:
        tpj = int(value[0]) * 3600 + int(value[1]) * 60
    else:
        raise Exception('Wrong input of time per job. Must be HH:mm or HH.')

    f.close()
    return max_jobs, max_wall_time, spm, tpj


def unfinish_jobs():
    unfinish = 0
    items =  os.listdir('.')
    for item in items:
        if os.path.isdir(item) and item.startswith('run_'):
            if not os.path.isfile(os.path.join(item, 'run.stop')):
                unfinish += 1
            else:  # remove .img file to save space
                os.chdir(item)
                if os.path.isfile(img_file):
                    os.remove(img_file)
                if os.path.isfile(mat_file):
                    os.remove(mat_file)
                os.chdir('..')
    return unfinish


if __name__ == "__main__":
    try:
        flog = open('bsub-run.log', 'w')
    except:
        raise Exception('Fail to create log file')

    with open('scan.ini', 'r') as f:
        lines = f.readlines()

    img_file = get_input(lines[0])
    if not os.path.isfile(img_file):
        raise Exception('Cannot find ' + img_file)

    mat_file = get_input(lines[1])
    if not os.path.isfile(mat_file):
        raise Exception('Cannot find ' + mat_file)

    fixed_estep = len(get_input(lines[3]).split(' '))
    random_estep = int(get_input(lines[4]).split(' ')[2])
    estep = fixed_estep + random_estep

    value = get_input(lines[5]).split(' ')
    xmin, xmax, dx = [int(i) for i in value]
    nx = (xmax - xmin) // dx + 1

    value = get_input(lines[6]).split(' ')
    zmin, zmax, dz = [int(i) for i in value]
    nz = (zmax - zmin) // dz + 1

    max_primary_M = int(get_input(lines[9])) / 1e6

    flog.write('Scan in the range X[{:d}, {:d}, {:d}] & Z[{:d}, {:d}, {:d}]\n'.format(xmin, xmax, dx, zmin, zmax, dz))

    run_start = time.time()
    flog.write('Starting at {:s}\n'.format(time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())))

    max_jobs, max_wall_time, spm, tpj = read_limits()
    task_id = 0
    x = xmin
    z = zmin
    cwd = os.getcwd()
    while time.time() - run_start < max_wall_time and z <= zmax:
        if unfinish_jobs() < max_jobs:
            run_name = 'run_{:04d}'.format(task_id)
            while os.path.isdir(run_name):
                task_id += 1
                run_name = 'run_{:04d}'.format(task_id)
            
            timestr = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())
            flog.write('    X={:d}, Z={:d} --> {:s}: '.format(x, z, run_name))

            os.mkdir(run_name)
            with open(os.path.join(run_name, 'scan.ini'), 'w') as f:
                nspot = int(tpj // (spm * max_primary_M * estep))
                if nspot <= nx:
                    lines[6] = 'Beam_Z = {:d} {:d} {:d}\n'.format(z, z, dz)

                    delta_x = dx * nspot
                    x2 = x + delta_x - 1
                    if x2 > xmax:  x2 = xmax
                    lines[5] = 'Beam_X = {:d} {:d} {:d}\n'.format(x, x2, dx)

                    x += delta_x
                    if x >= xmax:
                        x = xmin
                        z += dz

                else:
                    lines[5] = 'Beam_X = {:d} {:d} {:d}\n'.format(xmin, xmax, dx)
                    
                    dz_per_run = nspot // nx
                    delta_z = dz * dz_per_run
                    z2 = z + delta_z - 1
                    if z2 > zmax:  z2 = zmax
                    lines[6] = 'Beam_Z = {:d} {:d} {:d}\n'.format(z, z2, dz)

                    z += delta_z

                f.writelines(lines)
            
            shutil.copy(img_file, run_name)
            shutil.copy(mat_file, run_name)
            # shutil.copy('dosescan', run_name)
            os.chdir(run_name)

            # busb command to submit a job
            # submit = subprocess.check_output('bsub -G rpgrp -W 12:00 {:s}/dosescan'.format(cwd), shell=True)
            # flog.write('{:s}: {:s}'.format(timestr, submit.decode()))
            
            os.chdir('..')

        else:
            time.sleep(15)

        # refresh the limits to allow run-time changes
        max_jobs, max_wall_time, spm, tpj = read_limits()


    flog.write('Finished at {:s}\n'.format(time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())))
    if z > zmax:
        flog.write('  All jobs are submited\n')
    else:
        flog.write('  Time limit reaches.\n')
    flog.close()
