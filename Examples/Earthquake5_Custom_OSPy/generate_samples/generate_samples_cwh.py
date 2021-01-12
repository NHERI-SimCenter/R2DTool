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
# Joanna Zou
# Adam Zsarnóczay


import argparse, json
import numpy as np
import pandas as pd
import random
import os
# import matplotlib.pyplot as plt
from scipy import stats
from pelicun.uq_new import RandomVariable, RandomVariableRegistry

# generates building source file for cripple wall wood-frame model
def generate_samples_cwh(building_source_file, n_stories, cw_tag, constr_era, ext_mat):

    ### check values
    if n_stories <= 0:
        print("ERROR: number of stories (n_stories) must be nonzero")
    if cw_tag != 0 or cw_tag != 1:
        print("ERROR: cripple wall tag (cw_tag) must be 0 or 1")
    if constr_era != "pre-1950" or constr_era != "post-1950":
        print("ERROR: construction era (constr_era) must be 'pre-1950' or 'post-1950'")
    if ext_mat != "Wood Siding" or ext_mat != "Stucco":
        print("ERROR: exterior material type (ext_mat) must be 'Wood Siding' or 'Stucco'")


    ### load CSV file with the building information
    bldgs_df = pd.read_csv(building_source_file, header=0)
    num_samples = len(bldgs_df)


    ### generate LHS samples of continuous input parameters
    # create the RV registry that will handle all the random variables
    RV_reg = RandomVariableRegistry()

    # define the random variables
    RV_reg.add_RV(RandomVariable(name='materialQuality', distribution='uniform', theta=[0., 1.]))
    RV_reg.add_RV(RandomVariable(name='planArea', distribution='uniform', theta=[800., 2400.]))
    RV_reg.add_RV(RandomVariable(name='aspectRatio', distribution='uniform', theta=[0.4, 0.8]))
    RV_reg.add_RV(RandomVariable(name='damping', distribution='uniform', theta=[0.02, 0.1]))
    RV_reg.add_RV(RandomVariable(name='extDensity_x', distribution='uniform', theta=[0.5, 0.8]))
    RV_reg.add_RV(RandomVariable(name='extDensity_z', distribution='uniform', theta=[0.5, 0.8]))
    RV_reg.add_RV(RandomVariable(name='intDensity', distribution='uniform', theta=[0.06, 0.08]))
    RV_reg.add_RV(RandomVariable(name='extEcc_x', distribution='uniform', theta=[0.0, 0.5]))
    RV_reg.add_RV(RandomVariable(name='extEcc_z', distribution='uniform', theta=[0.0, 0.5]))
    RV_reg.add_RV(RandomVariable(name='intEcc_x', distribution='uniform', theta=[0.0, 0.5]))
    RV_reg.add_RV(RandomVariable(name='intEcc_z', distribution='uniform', theta=[0.0, 0.5]))
    RV_reg.add_RV(RandomVariable(name='floor1Weight', distribution='uniform', theta=[10., 15.5]))

    # assign floor weight parameters dependent on construction era
    if constr_era == "pre-1950":
        RV_reg.add_RV(RandomVariable(name='floor2Weight', distribution='uniform', theta=[15., 35.]))
        RV_reg.add_RV(RandomVariable(name='roofWeight', distribution='uniform', theta=[15., 30.]))
    elif constr_era == "post-1950":
        RV_reg.add_RV(RandomVariable(name='floor2Weight', distribution='uniform', theta=[10., 30.]))
        RV_reg.add_RV(RandomVariable(name='roofWeight', distribution='uniform', theta=[10., 25.]))

    # generate the random samples
    RV_reg.generate_samples(sample_size = num_samples, method="LHS_midpoint", seed=1)
    model_df = pd.DataFrame(RV_reg.RV_samples)


    ### set building archetype properties (discrete input parameters)
    # broadcast to total number of samples
    bldg_archetype = np.array([n_stories, cw_tag, constr_era, ext_mat])
    bldg_archetype = np.broadcast_to(bldg_archetype, (num_samples, bldg_archetype.shape[0]))
    archetype_df = pd.DataFrame(bldg_archetype, columns=["numStories", "crippleWallTag", "constructionEra", "exteriorMaterial"])


    # reassign value to default parameters (number of stories, plan area)
    bldgs_df['NumberofStories'] = archetype_df['numStories']
    bldgs_df['PlanArea'] = model_df['planArea']


    ### append model properties to the building source file
    bldgs_df = pd.concat([bldgs_df, archetype_df, model_df], axis=1)
    print(bldgs_df)
    bldgs_df.to_csv(building_source_file, index=False)

    print("building source file updated.")


if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('--buildingSourceFile')
    parser.add_argument('--num_stories', type=int, default=1)
    parser.add_argument('--cw_tag', type=int, default=0)
    parser.add_argument('--constr_era', default="pre-1950")
    parser.add_argument('--ext_mat', default="Wood Siding")
    args = parser.parse_args()

    generate_samples_cwh(args.buildingSourceFile, args.num_stories,
                   args.cw_tag, args.constr_era, args.ext_mat)
