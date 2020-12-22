# -*- coding: utf-8 -*-
#
# Copyright (c) 2018 Leland Stanford Junior University
# Copyright (c) 2018 The Regents of the University of California
#
# This file is part of the SimCenter Backend Applications
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# You should have received a copy of the BSD 3-Clause License along with
# this file. If not, see <http://www.opensource.org/licenses/>.
#
# Contributors:
# Adam Zsarnóczay
# Joanna J. Zou
#

import os
from sys import platform

import numpy as np
from math import pi, sqrt

if platform == "darwin":  # MACOS
    import openseespymac.opensees as ops
else:
    import openseespy.opensees as ops

from openseespy.opensees import *

# constants
node_tags = [0, 1]
height = 240. # in

#TODO: relax the fixed units
G = 386.1

def build_model(model_params):
    """
    Generates OpenSeesPy model of an elastic-perfectly plastic SDOF system and
    runs gravity analysis.

    Assumes that length is measured in inches and acceleration in in/s2

    Parameters
    ----------
    W: float
        Weight of the structure
    f_yield: float
        Yield stress of the material
    T1: float
        Fundamental period of the structure

    """

    W = model_params["W"]
    f_yield = model_params["f_yield"]
    T1 = model_params["T1"]
    m = W / G
    #print("m: " + str(m))

    # set model dimensions and deg of freedom
    ops.model('basic', '-ndm', 3, '-ndf', 6)

    # define nodes
    ops.node(node_tags[0], 0., 0., 0.)
    ops.node(node_tags[1], 0., 0., height)

    # define fixities
    ops.fix(node_tags[0], 1, 1, 1, 1, 1, 1)
    ops.fix(node_tags[1], 0, 0, 0, 1, 1, 1)

    # define bilinear (elastic-perfectly plastic) material
    steel01_tag = 100
    rigid_tag = 110
    K = m / (T1/(2*pi))**2.
    #print("K: " + str(K))
    ops.uniaxialMaterial('Steel01', steel01_tag, f_yield, K, 0.0001)
    ops.uniaxialMaterial('Elastic', rigid_tag, 1.e9)

    # define element
    element_tag = 1000
    ops.element('twoNodeLink', element_tag, node_tags[0], node_tags[1],
                '-mat', rigid_tag, steel01_tag, steel01_tag,
                '-dir', 1, 2, 3,
                '-orient', 0., 0., 1., 0., 1., 0., '-doRayleigh')

    # define mass
    ops.mass(node_tags[1], m, m, m, 0., 0., 0.)

    # define gravity loads
    ops.timeSeries('Linear', 1)
    ops.pattern('Plain', 101, 1)
    ops.load(node_tags[1], 0., 0., -W, 0., 0., 0.)

    # define damping based on first eigenmode
    damp_ratio = 0.05
    angular_freq = ops.eigen(1)[0]**0.5
    beta_k = 2 * damp_ratio / angular_freq
    ops.rayleigh(0., beta_k, 0., 0.)

    # run gravity analysis
    tol = 1e-8                          # convergence tolerance for test
    iter = 100                          # max number of iterations
    nstep = 100                         # apply gravity loads in 10 steps
    incr = 1./nstep                     # first load increment

    # analysis settings
    constraints('Transformation')       # enforce boundary conditions using transformation constraint handler
    numberer('RCM')                     # renumbers dof's to minimize band-width (optimization)
    system('BandGeneral')               # stores system of equations as 1D array of size bandwidth x number of unknowns
    test('EnergyIncr', tol, iter, 0)    # tests for convergence using dot product of solution vector and norm of right-hand side of matrix equation
    algorithm('Newton')                 # use Newton's solution algorithm: updates tangent stiffness at every iteration
    integrator('LoadControl', incr)     # determine the next time step for an analysis # apply gravity in 10 steps
    analysis('Static')                  # define type of analysis, static or transient
    analyze(nstep)                      # perform gravity analysis

    # after gravity analysis, change time and tolerance for the dynamic analysis
    loadConst('-time', 0.0)


def run_analysis(GM_dt, GM_npts, TS_List, EDP_specs):
    """
    Run dynamic analysis for a time history and return a dictionary of envelope EDPs.

    Assumes that length is measured in inches and acceleration in in/s2

    Parameters
    ----------
    GM_dt: float
        Time step of time series
    GM_npts: float
        Number of points in time series
    TS_List: float
        1x2 list where first component is a list of accelerations in the X
        direction, the second component is a list of accelerations in the Y
        direction.
    EDP_specs: dict

    """

    # define parameters for dynamic analysis
    dt = GM_dt          # time increment for analysis

    GMX = TS_List[0]    # list of GM accelerations in X direction
    GMY = TS_List[1]    # list of GM accelerations in Y direction

    driftLimit = 0.20   # interstory drift limit indicating collapse
    tol = 1.e-08        # tolerance criteria to check for convergence
    maxiter = 30        # max number of iterations to check
    subSteps = 2        # number of subdivisions in cases of ill convergence

    # pad shorter record with zeros (free vibration) such that two horizontal records are the same length
    nsteps = max(len(GMX), len(GMY))
    for GM in [GMX, GMY]:
        if len(GM) < nsteps:
            diff = nsteps - len(GM)
            GM.extend(np.zeros(diff))

    # initialize dictionary of envelope EDPs
    envelopeDict = {}
    for edp in EDP_specs:
        envelopeDict[edp] = {}
        for loc in EDP_specs[edp]:
            envelopeDict[edp][loc] = np.zeros(len(EDP_specs[edp][loc])).tolist()
    #print(envelopeDict)

    # initialize dictionary of time history EDPs
    time_analysis = np.zeros(nsteps * 5)
    acc_history = {
        0 : {
            1: time_analysis.copy(),
            2: time_analysis.copy()
        },
        1 : {
            1: time_analysis.copy(),
            2: time_analysis.copy()
        }
    }

    ops.wipeAnalysis()

    ops.constraints('Transformation')       # handles boundary conditions based on transformation equation method
    ops.numberer('RCM')                     # renumber dof's to minimize band-width (optimization)
    ops.system('UmfPack')                   # constructs sparse system of equations using UmfPack solver
    ops.test('NormDispIncr', tol, maxiter)  # tests for convergence using norm of left-hand side of matrix equation
    ops.algorithm('NewtonLineSearch')       # use Newton's solution algorithm: updates tangent stiffness at every iteration
    ops.integrator('Newmark', 0.5, 0.25)    # Newmark average acceleration method for numerical integration
    ops.analysis('Transient')               # define type of analysis: time-dependent

    # initialize variables
    levels = 2
    maxDiv = 1024
    minDiv = subSteps
    step = 0
    ok = 0
    breaker = 0
    count = 0

    while step < nsteps and ok == 0 and breaker == 0:
        step = step + 1  # take 1 step
        ok = 2
        div = minDiv
        length = maxDiv
        while div <= maxDiv and length > 0 and breaker == 0:
            stepSize = dt / div
            # perform analysis for one increment; will return 0 if no convergence issues
            ok = ops.analyze(1, stepSize)
            if ok == 0:
                count = count + 1
                length = length - maxDiv / div

                # check if drift limits are satisfied
                # check X direction drifts (direction 1)
                drift_x = abs(nodeDisp(node_tags[1], 1) -
                              nodeDisp(node_tags[0], 1)) / height
                if drift_x >= driftLimit:
                    breaker = 1

                # check Y direction drifts (direction 2)
                drift_y = abs(nodeDisp(node_tags[1], 2) -
                              nodeDisp(node_tags[0], 2)) / height
                if drift_y >= driftLimit:
                    breaker = 1

                # save parameter values in recording dictionaries at every step
                time_analysis[count] = time_analysis[count - 1] + stepSize

                envelopeDict['PID'][1][0] = max(drift_x,
                                                envelopeDict['PID'][1][0])
                envelopeDict['PID'][1][1] = max(drift_y,
                                                envelopeDict['PID'][1][1])

                for floor in [0, 1]:
                    for dof in [1, 2]:
                        acc_history[floor][dof][count] = nodeAccel(
                            node_tags[floor], dof)

            else:
                div = div * 2
                print("Number of increments increased to ", str(div))

        # end analysis once drift limit has been reached
        if breaker == 1:
            ok = 1
            print("Collapse drift has been reached")

    print("Number of analysis steps completed: {}".format(count))

    # remove extra zeros from the end of the time history
    time_analysis = time_analysis[1:count + 1]

    # generate time array from recording
    time_record = np.linspace(0, nsteps * dt, num=nsteps, endpoint=False)

    # remove extra zeros from accel time history, add GM to obtain absolute a
    # acceleration, and record envelope value
    GMX_interp = np.interp(time_analysis, time_record, GMX)
    GMY_interp = np.interp(time_analysis, time_record, GMY)
    for level in range(0, levels):
        # X direction
        envelopeDict['PFA'][level][0] = max(abs(np.asarray(
            acc_history[level][1][1:count + 1]) + GMX_interp))
        # Y direction
        envelopeDict['PFA'][level][1] = max(abs(np.asarray(
            acc_history[level][2][1:count + 1]) + GMY_interp))

    return envelopeDict