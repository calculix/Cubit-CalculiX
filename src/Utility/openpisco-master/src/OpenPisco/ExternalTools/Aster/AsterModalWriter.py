# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
"""AsterModalWriter

This is the dedicated writer file for the modal analysis. We refer to the documentation of AsterCommonWriter for a more general perspective.
"""
from typing import TextIO,Dict,Iterable,Tuple

import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter 

def WriteMaterialParametersInput(writeFile:TextIO,materials:Iterable[Tuple[str,Dict]]):
    """Write material parameters input for modal analysis in file. 
    Young modulus ("young"), poisson ratio ("poisson") and density ("density") are mandatory for each material. 
    
    Example:
    import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
    writeFile = AsterCommonWriter.OpenAsterParamFile("myFile.param")
    material1={"young":210000.0,"poisson":0.3,"density":7.85e-09}
    materials = [['AllZones',material1],]
    WriteMaterialParametersInput(writeFile,materials)
    AsterCommonWriter.CloseAsterParamFile(writeFile)

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    materials : Iterable[Iterable[str,Dict]]
        materials description (element tag followed by parameters value for each material)
    """
    affe_mater=[]
    for num_mater,(tagname,material) in enumerate(materials):
        AsterCommonWriter.WriteVariable(writeFile,variableName='young_modulus_'+str(num_mater),variableValue=material["young"])
        AsterCommonWriter.WriteVariable(writeFile,variableName='poisson_number_'+str(num_mater),variableValue=material["poisson"])
        AsterCommonWriter.WriteVariable(writeFile,variableName='density_'+str(num_mater),variableValue=material["density"])
        
        value={}
        value["E"]="young_modulus_"+str(num_mater)
        value["NU"]="poisson_number_"+str(num_mater)
        value["RHO"]="density_"+str(num_mater)

        writeFile.write('mater_'+str(num_mater)+' = DEFI_MATERIAU(ELAS=_F('+','.join([ key+"="+value[key] for key in value.keys()])+'))\n')
        if tagname=='AllZones':
            affe_mater.append("_F(MATER=("+'mater_'+str(num_mater)+", ),TOUT=\'OUI\')")
        else:
            affe_mater.append("_F(MATER=("+'mater_'+str(num_mater)+", ),GROUP_MA=(\""+str(tagname)+"\",))")
    writeFile.write('materials = {\'AFFE\' :'+str(affe_mater).replace("\'_","_").replace(")\'",")").replace("\"_","_").replace(")\"",")").replace("\"\'","\"")+',}\n')
