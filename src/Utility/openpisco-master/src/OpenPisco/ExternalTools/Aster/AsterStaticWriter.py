# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
"""AsterStaticWriter

This is the dedicated writer file for the static analysis. We refer to the documentation of AsterCommonWriter for a more general perspective.
"""
from typing import TextIO

def WriteModelisation(writeFile:TextIO,dimensionality:int,planeStress:bool=True):
    """Write the modelisation in file

    Example:
    import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
    writeFile = AsterCommonWriter.OpenAsterParamFile("myFile.param")
    WriteModelisation(writeFile,dimensionality=3)
    AsterCommonWriter.CloseAsterParamFile(writeFile)

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    dimensionality : int
        problem dimensionality, correspond to dimension of the mesh (either 2 or 3)
    planeStress : bool, optional
        plane stress assumption for 2D cases only (irrelevant in 3D), by default True
    """
    planeStressDict={True:'C_PLAN',False:'D_PLAN'}
    asterModelisation = {"modelisationType": {"2D":planeStressDict[planeStress],"3D":"3D"}}
    asterModelisationKeyword = asterModelisation["modelisationType"][str(dimensionality)+"D"]
    writeFile.write("modelisation = {'MODELISATION':('"+asterModelisationKeyword+"', ),'PHENOMENE':'MECANIQUE','TOUT':'OUI'}\n")
