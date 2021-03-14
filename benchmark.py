#!/usr/bin/env python3

import os
import platform as plat
import ctypes as ct
import numpy as np
import matplotlib.pyplot as plt

path = 'build/librmscalc.so'

class RMS:
    def __init__(self):
        libname = os.path.dirname(os.path.abspath(__file__)) + os.path.sep
        libname += path.replace('/', os.path.sep)

        self.lib = ct.cdll.LoadLibrary(libname)

        self.lib.rmscalc.restype = ct.c_uint16
        self.lib.rmscalc.argtypes = [ct.c_uint16, ct.c_uint16]

        self.lib.rmscalc_sp.restype = ct.c_uint16
        self.lib.rmscalc_sp.argtypes = [ct.c_uint16, ct.c_uint16]

    def rmscalc(self, sample, freq):
        return self.lib.rmscalc(sample, freq)

    def rmscalc_sp(self, sample, freq):
        return self.lib.rmscalc_sp(sample, freq)

if __name__ == "__main__":
    rms = RMS()
    f = 55.0
    t = np.arange(0, 1, 1/1500)
    v = np.sin(2 * np.pi * f * t)
    # v[v>0] = 1
    # v[v<0] = -1
    k = ((2**15) - 1) / 600

    for a in range(220, 221):
        vs = v * a * k    
        r = []
        p = []
        for i in vs:
            tmp = rms.rmscalc(np.uint16(i), np.uint16(f * 10))
            r.append((tmp / k))
            tmp = rms.rmscalc_sp(np.uint16(i), np.uint16(f * 10))
            p.append(tmp/k)

        rrms = a * np.sqrt(2) / 2
        crms = np.average(r[120:])
        err = 100 * np.abs(rrms - crms) / rrms

        prms = np.average(p[120:])
        perr = 100 * np.abs(rrms - prms) / rrms

        # print('Real: {0:.1f}, RMS: {1:.1f}, Error: {2:.1f}'.format(rrms, crms, err))
        #print(np.average(r[120:]))
        # print('Real: {0:.1f}, RMS: {1:.1f}, Error: {2:.1f}, Error: {3:.1f}'.format(rrms, crms, err, perr))

    plt.plot(t, r)
    plt.plot(t, p)
    plt.show()