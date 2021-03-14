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

        self.lib.rmscalc.restype = ct.c_int16
        self.lib.rmscalc.argtypes = [ct.c_int16, ct.c_int16]

        self.lib.rmscalc_sp.restype = ct.c_int16
        self.lib.rmscalc_sp.argtypes = [ct.c_int16, ct.c_int16]

        self.lib.rmscalc_ex.restype = ct.c_int16
        self.lib.rmscalc_ex.argtypes = [ct.c_int16]

    def rmscalc(self, sample, freq):
        return self.lib.rmscalc(sample, freq)

    def rmscalc_sp(self, sample, freq):
        return self.lib.rmscalc_sp(sample, freq)

    def rmscalc_ex(self, sample):
        return self.lib.rmscalc_ex(sample)

if __name__ == "__main__":
    rms = RMS()
    f = 50.0
    t = np.arange(0, 1, 1/1500)
    v = .5 * np.sin(2 * np.pi * f * t) + .5 * np.random.normal(0,.1,len(t))
    # v[v>0] = 1
    # v[v<0] = -1
    k = ((2**15) - 1) / 600

    for a in range(220, 221):
        vs = v * a * k    
        r = []
        p = []
        e = []
        for i in vs:
            tmp = rms.rmscalc(np.uint16(i), np.uint16(f * 10))
            r.append((tmp / k))
            tmp = rms.rmscalc_sp(np.uint16(i), np.uint16(f * 10))
            p.append(tmp/k)
            tmp = rms.rmscalc_ex(np.uint16(i))
            e.append(tmp/k)

        rrms = a * np.sqrt(2) / 2
        crms = np.average(r[120:])
        err = 100 * np.abs(rrms - crms) / rrms

        prms = np.average(p[120:])
        perr = 100 * np.abs(rrms - prms) / rrms

        erms = np.average(e[120:])
        eerr = 100 * np.abs(rrms - erms) / rrms

        # print('Real: {0:.1f}, RMS: {1:.1f}, Error: {2:.1f}'.format(rrms, crms, err))
        #print(np.average(r[120:]))
        # print('Real: {0:.1f}, RMS: {1:.1f}, Error: {2:.1f}, Error: {3:.1f}'.format(rrms, crms, err, perr))

    plt.plot(t, r)
    plt.plot(t, p)
    plt.plot(t, e)
    plt.show()