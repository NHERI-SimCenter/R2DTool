
Earthquake - Alameda
====================

+-----------------+----------------------------------------------------+
| Download files  | :github:`Github <Examples/Earthquake2_Alameda/>`   |
+-----------------+----------------------------------------------------+



#. **VIZ**
    
   .. figure:: figures/r2dt-0002-VIZ.png
      :width: 600px
      :align: center


#. **GI** Next, the general information panel is used to broadly characterize the problem at hand.
    
   .. figure:: figures/r2dt-0002-GI.png
      :width: 600px
      :align: center


#. **HAZ** Now in the hazard panel, the **User Specified Ground Motions** option is selected which allows for the use of pre-generated earthquake scenarios. The following figure shows the relevant example files which are now entered in this pane.
    
   .. figure:: figures/r2dt-0002-HAZ.png
      :width: 600px
      :align: center

#. **ASD** In the asset definition panel, the path to the ``all_bldgs.csv`` file is specified. Once this file is loaded, the user can select which particular assets will be included in the analysis by entering a valid range in the form and clicking **Select**. For this example, the range **1-20** is used to include all buildings.

   .. figure:: figures/r2dt-0002-ASD.png
      :width: 600px
      :align: center

#. **HTA** Next, a hazard mapping algorithm is specified using the **Nearest Neighbour** method and the **SimCenterEvent** application, which are configured as show in the following figure with **1000** samples in **4** neighborhoods. 

   .. figure:: figures/r2dt-0002-HTA.png
      :width: 600px
      :align: center

#. **MOD** In the building modeling panel, simply leave the first dropdown box set to **None**.


#. **ANA** In the analysis panel, **IMasEDP** is selected from the primary dropdown.

   .. figure:: figures/r2dt-0002-ANA.png
      :width: 600px
      :align: center

#. **DL** The damage and loss panel is now used to configure the **Pelicun** backend. The **HAZUS MH EQ IM** damage and loss method is selected and configured as shown in the following figure:

   .. figure:: figures/r2dt-0002-DL.png
      :width: 600px
      :align: center

#. **UQ**

   .. figure:: figures/r2dt-0002-UQ.png
      :width: 600px
      :align: center

#. **RV** 

   The random variable panel will be left empty for this example.

#. **RES** 

   .. figure:: figures/r2dt-0002-RES.png
      :width: 600px
      :align: center

