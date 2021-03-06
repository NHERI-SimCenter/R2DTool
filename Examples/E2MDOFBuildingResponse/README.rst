
E2 - MDOF Building Response
===========================

+-----------------+-----------------------------------------------------+
| Download files  | :github:`Github <Examples/E2MDOFBuildingResponse/>` |
+-----------------+-----------------------------------------------------+

This example uses ground motions from the 2018 earthquake in Anchorage, AK, to characterize the response of buildings with an idealized MDOF building model through story-level Engineering Demand Parameters (EDP). Exploiting the refinement in EDPs, a HAZUS-based story-level assessment is employed to evaluate the performance of 510 buildings.


#. **VIZ** This example shows simulation of the magnitude 7 earthquake occurred in Anchorage in 2018. The damage and loss of a sample of buildings in the region are estimated.  It is important to note that the results are based on an approximate characterization of the ground motions and preliminary buildings data that has not been curated or verified thoroughly yet. The results presented herein are only for demonstrating the use of R2DTool and do not serve as an accurate representation of the real losses resulting from the earthquake.

   .. figure:: figures/r2dt-0003-VIZ.png
      :width: 600px
      :align: center


#. **GI** First, the unit system and asset type are prescribed in this panel, and we're interested in the building engineering demand parameters, damage measures, and the resulting decision variables.

   .. figure:: figures/r2dt-0003-GI.png
      :width: 600px
      :align: center


#. **HAZ** 12 recorded ground motions cross Anchorage (from the Center for Engineering Strong Motion Data, CESMD) are used as the ground acceleration time history inputs in this example. The PGA values of these 12 records range from 0.12 g to 0.81 g.

   .. figure:: figures/r2dt-0003-HAZ.png
      :width: 600px
      :align: center


#. **ASD** In the asset definition panel, the path to the ``AnchorageBuilding.csv`` file is specified. Once this file is loaded, the user can select which particular assets will be included in the analysis by entering a valid range (e.g., 1-50) in the form and clicking **Select**. The ``AnchorageBuilding.csv`` includes parameters for the damage and loss assessment (i.e., number of stories, year of built, occupancy class, structure type, and plan area) for more than 80,000 buildings in the community.

   .. figure:: figures/r2dt-0003-ASD.png
      :width: 600px
      :align: center


#. **HTA** Next, a hazard mapping algorithm is specified using the **Nearest Neighbour** method and the **SimCenterEvent** application, which are configured as show in the following figure with **3** samples in **4** neighborhoods, i.e., randomly sampling 5 ground motions from the nearest four stations (each station has one ground motion recording specified in the **HAZ**).

   .. figure:: figures/r2dt-0003-HTA.png
      :width: 600px
      :align: center


#. **MOD** In the modeling panel, the **MDOF-LU** method is used to create Multi-Degree-Of-Freedom (MDOF) nonlinear shear building model from the input ``AnchorageBuilding.csv``. Following the HAZUS EQ Technical Manual Chapter 5, a hysteretic nonlinear material is defined for each story with a story shear and displacement relationship with the initial stiffness, over-strength ratio, hardening ratio, and degradation factor. These parameters are stored in the ``HazusData.txt`` for different building design levels (e.g., high-, moderate-, or pre-code) which is now primarily based on the built year of the structure.

   .. figure:: figures/r2dt-0003-MOD.png
      :width: 600px
      :align: center


#. **ANA** In the analysis panel, **OpenSees** is selected from the primary dropdown.

   .. figure:: figures/r2dt-0003-ANA.png
      :width: 600px
      :align: center


#. **DL** The damage and loss panel is now used to configure the **Pelicun** backend. The **HAZUS MH EQ** damage and loss method is selected and configured as shown in the following figure:

   .. figure:: figures/r2dt-0003-DL.png
      :width: 600px
      :align: center

..
   #. **UQ**

      .. figure:: figures/r2dt-0003-UQ.png
         :width: 600px
         :align: center

..
   #. **RV**

      .. figure:: figures/r2dt-0003-RV.png
         :width: 600px
         :align: center


#. **RES** The analysis outputs for the selected 50 buildings are show in the figure below. The buildings are mostly likely in moderate damage states (Damage State 2 or 3 per HAZUS) with the non-structural damage would dominate the economic losses. The repair costs range from 1% to 7% of the total replacement costs, and the repair time range from 1 to 20 days.

   .. figure:: figures/r2dt-0003-RES.png
      :width: 600px
      :align: center
