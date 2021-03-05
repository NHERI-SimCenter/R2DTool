
Physics-based Ground Motions
============================

+-----------------+--------------------------------------------------------+
| Download files  | :github:`Github <Examples/Earthquake4_Alameda_SW4/>`   |
+-----------------+--------------------------------------------------------+

This example features ground motion acceleration time histories simulated using the SW4 software and a detailed geophysical model of the San Francisco Bay Area by the Lawrence Livermore National Laboratory (https://doi.org/10.1785/0220180261). The ground motions are used to investigate the impact of a Mw7.0 earthquake on the Hayward fault on the city of Berkeley. Engineering Demand Parameters are simulated with an idealized MDOF building model; building performance is evaluated at the story-level based on the HAZUS earthquake damage and loss assessment methodology.


.. note::
   This example uses simulated ground motion time histories from the Lawrence Livermore National Lab. Due to size constraints, the time histories are not bundled with R2D, but they are available at https://berkeley.box.com/s/65113pqclc2j29ve9alita5kr7q2jnwc . After downloading the zip file, extract its contents to the SW4 folder under input_data.


Modeling Procedure
------------------


#. **GI**
    
   .. figure:: figures/r2dt-0004-GI.png
      :width: 600px
      :align: center


#. **HAZ**
    
   .. figure:: figures/r2dt-0004-HAZ.png
      :width: 600px
      :align: center

#. **ASD** 

   .. figure:: figures/r2dt-0004-ASD.png
      :width: 600px
      :align: center

#. **HTA** Next, a hazard mapping algorithm is specified using the **Nearest Neighbour** method and the **SimCenterEvent** application, which are configured as show in the following figure with **3** samples in **4** neighborhoods. 

   .. figure:: figures/r2dt-0004-HTA.png
      :width: 600px
      :align: center

#. **MOD** 

   .. figure:: figures/r2dt-0004-MOD.png
      :width: 600px
      :align: center


#. **ANA** In the analysis panel, **OpenSees** is selected from the primary dropdown.

   .. figure:: figures/r2dt-0004-ANA.png
      :width: 600px
      :align: center


#. **DL** 

   .. figure:: figures/r2dt-0004-DL.png
      :width: 600px
      :align: center

#. **UQ** 

   .. figure:: figures/r2dt-0004-UQ.png
      :width: 600px
      :align: center

#. **RV** 

   .. figure:: figures/r2dt-0004-RV.png
      :width: 600px
      :align: center

#. **RES** 

   .. figure:: figures/r2dt-0004-RES.png
      :width: 600px
      :align: center

