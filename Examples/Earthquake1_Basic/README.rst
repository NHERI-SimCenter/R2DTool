
Basic Earthquake
================

+-----------------+----------------------------------------------------+
| Download files  | :github:`Github <Examples/Earthquake1_Basic/>`     |
+-----------------+----------------------------------------------------+



Required Files
--------------

#. :github:`cantilever_light.py <Examples/Earthquake1_Basic/input_data/model/cantilever_light.py>`
   This Python script defines the following functions which use OpenSeesPy to construct a structural model.


   .. automodule:: cantilever_light
      :members:


Modeling Procedure
------------------

The procedure for this example can be configured through the R2D interface by sequentially entering the following parameters into the respective panels:


#. **VIZ** 
   
   .. figure:: figures/r2dt-0001-VIZ.png
      :width: 600px
      :align: center


#. **GI** Next, the general information panel is used to broadly characterize the problem at hand.
   
   .. figure:: figures/r2dt-0001-GI.png
      :width: 600px
      :align: center


#. **HAZ** Now in the hazard panel, the **User Specified Ground Motions** option is selected which allows for the use of pre-generated earthquake scenarios. The following figure shows the relevant example files which are now entered in this pane.
    
   .. figure:: figures/r2dt-0001-HAZ.png
      :width: 600px
      :align: center


#. **ASD** In the asset definition panel, the path to the ``input_params.csv`` file is specified. Once this file is loaded, the user can select which particular assets will be included in the analysis by entering a valid range in the form and clicking **Select**.

   .. figure:: figures/r2dt-0001-ASD.png
      :width: 600px
      :align: center


#. **HTA** 

   .. figure:: figures/r2dt-0001-HTA.png
      :width: 600px
      :align: center


#. **MOD** 

   .. figure:: figures/r2dt-0001-MOD.png
      :width: 600px
      :align: center


#. **ANA** 

   .. figure:: figures/r2dt-0001-ANA.png
      :width: 600px
      :align: center

#. **DL** 

   .. figure:: figures/r2dt-0001-DL.png
      :width: 600px
      :align: center

#. **UQ** 

   .. figure:: figures/r2dt-0001-UQ.png
      :width: 600px
      :align: center

#. **RV** 

   For this problem, the **RV** panel will be left empty.


#. **RES** 

   .. figure:: figures/r2dt-0001-RES.png
      :width: 600px
      :align: center




