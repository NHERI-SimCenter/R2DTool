
E4 - OpenSeesPy FEM
===================


+-----------------+--------------------------------------------------+
| Download files  | :github:`Github <Examples/E4OpenSeesPyFEM/>`     |
+-----------------+--------------------------------------------------+


Advanced response estimation is enabled in this example through custom scripts that generate a finite element model for a building using the information available in the building inventory. A template script is provided that creates a building-specific cantilever and performs the response-history analysis to obtain EDPs. The hazard is described by empirical ground motion records from the PEER NGA West2 database. The buildings are located in Albany, CA, and their damage and loss assessment is based on the HAZUS methodology. This example includes 81 buildings and 5 are selected for analysis.

Required Files
--------------

#. :github:`cantilever_light.py <Examples/E4OpenSeesPyFEM/input_data/model/cantilever_light.py>`
   This Python script defines the following functions which use OpenSeesPy to construct a structural model.


   .. automodule:: cantilever_light
      :members:


Modeling Procedure
------------------

This example is a small-scale regional earthquake risk assessment which performs response simulation and damage/loss estimation for a group of 20 wood buildings under a pseudo earthquake scenario. The procedure for this example can be configured through the R2D interface by sequentially entering the following parameters into the respective panels:


#. **VIZ** The 20 buildings are distributed in space in a :math:`4 \times 5` grid in Northern California.

   .. figure:: figures/r2dt-0001-VIZ.png
      :width: 600px
      :align: center


#. **GI** Next, the general information panel is used to broadly characterize the problem at hand. In this example, the imperial force and length units are used, and we're interested in the building engineering demand parameters (e.g., peak story drift ratio, peak floor acceleration), damage measures, and the resulting decision variable (e.g., expected replacement cost).

   .. figure:: figures/r2dt-0001-GI.png
      :width: 600px
      :align: center


#. **HAZ** Now in the hazard panel, the **User Specified Ground Motions** option is selected which allows for the use of pre-generated earthquake scenarios. The following figure shows the relevant example files which are now entered in this pane. These ground motion records are selected for stations in a grid.


   .. figure:: figures/r2dt-0001-HAZ.png
      :width: 600px
      :align: center


#. **ASD** In the asset definition panel, the path to the ``input_params.csv`` file is specified. Once this file is loaded, the user can select which particular assets will be included in the analysis by entering a valid range in the form and clicking **Select**. The ``input_params.csv`` includes parameters for the damage and loss assessment (i.e., number of stories, year of built, occupancy class, structure type, plan area, and replacement cost) are specified.

   .. figure:: figures/r2dt-0001-ASD.png
      :width: 600px
      :align: center


#. **HTA** Next, a hazard mapping algorithm is specified using the **Nearest Neighbour** method and the **SimCenterEvent** application, which are configured as show in the following figure with **5** samples in **4** neighborhoods, i.e., randomly sampling 5 ground motions from the nearest four stations (each station has a set of records specified in the **HAZ**).

   .. figure:: figures/r2dt-0001-HTA.png
      :width: 600px
      :align: center


#. **MOD** In the modeling panel, the ``cantilever_light.py`` file is specified in the **Input Script** field and a DOF scheme is defined as shown in the following figure. This example uses the OpenSeesPyInput modeling application. The buildings are modeled as elastic-perfectly plastic single-degree-of-freedom (SDOF) systems defined by three input model parameters: the weight, yield strength, and fundamental period.

   .. figure:: figures/r2dt-0001-MOD.png
      :width: 600px
      :align: center


#. **ANA** In the analysis panel, **OpenSeesPy** is selected from the primary dropdown.

   .. figure:: figures/r2dt-0001-ANA.png
      :width: 600px
      :align: center

#. **DL** The damage and loss panel is now used to configure the **Pelicun** backend. The **HAZUS MH EQ** damage and loss method is selected and configured as shown in the following figure:

   .. figure:: figures/r2dt-0001-DL.png
      :width: 600px
      :align: center



#. **RES** The analysis outputs for the selected two buildings are show in the figure below. In the given earthquake scenario, the two buildings are mostly likely in no damage or minor damage (Damage State 1 or 2 per HAZUS) with about 1% expected loss ratios.

   .. figure:: figures/r2dt-0001-RES.png
      :width: 600px
      :align: center

