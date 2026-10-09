# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
"""AsterThermoElasticWriter

This is the dedicated writer file for the thermoelastic analysis. We refer to the documentation of AsterCommonWriter for a more general perspective.
"""
from typing import TextIO,Dict

from OpenPisco.ExternalTools.Aster.AsterCommonWriter import WriteVariable

def WriteMecaModelisation(writeFile:TextIO,dimensionality:int):
    """Write mechanical modelisation in file

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    dimensionality : int
        problem dimensionality, correspond to dimension of the mesh (either 2 or 3)
    """
    asterModelisationKeyword={2:'D_PLAN',3:'3D'}[dimensionality]
    writeFile.write("modelisation_meca = {'MODELISATION':('"+asterModelisationKeyword+"', ),'PHENOMENE':'MECANIQUE','TOUT':'OUI'}\n")

def WriteThermalModelisation(writeFile:TextIO,dimensionality:int):
    """Write thermal modelisation in file

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    dimensionality : int
        problem dimensionality, correspond to dimension of the mesh (either 2 or 3)
    """
    asterModelisationKeyword={2:'PLAN',3:'3D'}[dimensionality]
    writeFile.write("modelisation_thermal = {'MODELISATION':('"+asterModelisationKeyword+"', ),'PHENOMENE':'THERMIQUE','TOUT':'OUI'}\n")

def WriteMaterialParametersInput(writeFile:TextIO,materials:Dict):
    """Write material parameters input in file
    Young modulus ("young"), poisson ratio ("poisson"), thermal conductivity ("lambda"), 
    density ("rho") and thermal capacity ("cp"), thermal expansion coefficient ("thermalExpansion") and
    reference temperature ("referenceTemperature") are mandatory for each material. 

    Example:
    import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
    writeFile = AsterCommonWriter.OpenAsterParamFile("myFile.param")
    materialProperties={"lambda":50.0,"rho":7850.0,"cp":1046.0,"young":210000.0,"poisson":0.3,"thermalExpansion":11E-6,"referenceTemperature":20.0}
    materials = [['AllZones',materialProperties]]
    WriteMaterialParametersInput(writeFile,material)
    AsterCommonWriter.CloseAsterParamFile(writeFile)

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    materials : Dict
        materials description (element tag followed by parameters value for each material)
    """
    affeMater=[]

    for numMater,(tagname,material) in enumerate(materials):
        WriteVariable(writeFile,variableName='lambda_'+str(numMater),variableValue=material["lambda"])
        rho_cp = material["rho"]*material["cp"]
        WriteVariable(writeFile,variableName='rho_cp_'+str(numMater),variableValue=rho_cp)
        WriteVariable(writeFile,variableName='young_modulus_'+str(numMater),variableValue=material["young"])
        WriteVariable(writeFile,variableName='poisson_number_'+str(numMater),variableValue=material["poisson"])
        WriteVariable(writeFile,variableName='alpha_'+str(numMater),variableValue=material["thermalExpansion"])
        WriteVariable(writeFile,variableName='reference_temperature_'+str(numMater),variableValue=material["referenceTemperature"])
        value={}
        value["LAMBDA"]="lambda_"+str(numMater)
        value["RHO_CP"]="rho_cp_"+str(numMater)
        valueElas = {}
        valueElas["E"]="young_modulus_"+str(numMater)
        valueElas["NU"]="poisson_number_"+str(numMater)
        valueElas["ALPHA"]="alpha_"+str(numMater)

        writeFile.write('mater_'+str(numMater)+' = DEFI_MATERIAU(THER=_F('+','.join([ key+"="+value[key]
        for key in value.keys()])+'), ELAS=_F('+','.join([ key+"="+valueElas[key]
        for key in valueElas.keys()])+'))\n')
        if tagname=='AllZones':
            affeMater.append("_F(MATER=("+'mater_'+str(numMater)+", ),TOUT=\'OUI\')")
        else:
            affeMater.append("_F(MATER=("+'mater_'+str(numMater)+", ),GROUP_MA=(\""+str(tagname)+"\",))")
    writeFile.write('materials = {\'AFFE\' :'+str(affeMater).replace("\'_","_").replace(")\'",")").replace("\"_","_").replace(")\"",")").replace("\"\'","\"")+',}\n')
