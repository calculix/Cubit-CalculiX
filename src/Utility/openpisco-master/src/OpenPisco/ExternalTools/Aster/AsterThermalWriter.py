# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
"""AsterThermalWriter

This is the dedicated writer file for the thermal analysis. We refer to the documentation of AsterCommonWriter for a more general perspective.
"""
from typing import TextIO,Iterable,Dict,Tuple

def WriteModelisation(writeFile:TextIO,dimensionality:int):
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
    """
    asterModelisationKeyword={2:'PLAN',3:'3D'}[dimensionality]
    writeFile.write("modelisation = {'MODELISATION':('"+asterModelisationKeyword+"', ),'PHENOMENE':'THERMIQUE','TOUT':'OUI'}\n")

def WriteConvectionParametersInput(writeFile:TextIO,convection:Iterable[Tuple[str,Dict]]):
    """Write convection parameter in file
    Heat transfer coefficient ("h") and external temperature ("temp_ext") are mandatory for each zone. 

    Example:
    import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
    writeFile = AsterCommonWriter.OpenAsterParamFile("myFile.param")
    convection= [['X0',{"h":50.0,"temp_ext":500.0}]]
    WriteConvectionParametersInput(writeFile,convection)
    AsterCommonWriter.CloseAsterParamFile(writeFile)
    
    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    convection : Iterable[Tuple[str,Dict]]
        convection description on suitable boundary tags. On each tag, the external temperature ("temp_ext") and the convection coefficient ("h") should be provided
    """
   
    writeFile.write("convection = {'ECHANGE' : [\n")
    for tagname,params in convection:
        try:
            writeFile.write("          _F(COEF_H="+str(params["h"])+", TEMP_EXT="+str(params["temp_ext"])+", GROUP_MA=('"+str(tagname)+"', )),\n")
        except KeyError as e:
            raise Exception("Both the external temperature and the convection coefficient should be provided at boundary "+tagname) from e
    writeFile.write("],\n")
    writeFile.write("}\n")

def WriteSourceParametersInput(writeFile:TextIO,source:Iterable[Tuple[str,Dict]]):
    """Write source parameters input in file
    Source term value ("source") is mandatory for each zone. 

    Example:
    import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
    writeFile = AsterCommonWriter.OpenAsterParamFile("myFile.param")
    source = [['AllZones',{"source":1.0}]]
    WriteSourceParametersInput(writeFile,source)
    AsterCommonWriter.CloseAsterParamFile(writeFile)

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    source : Iterable[Tuple[str,Dict]]
        source description on suitable boundary tags
    """
    writeFile.write("source = {'SOURCE' : [\n")
    for tagname,params in source:
        try:
            if tagname=='AllZones':
                writeFile.write("          _F(SOUR="+str(params["source"])+", TOUT=\'OUI\')")
            else:
                writeFile.write("          _F(SOUR="+str(params["source"])+", GROUP_MA=('"+str(tagname)+"', )),\n")
        except KeyError as e:
            raise Exception("Source term is mandatory for source term for boundary "+tagname) from e
    writeFile.write("],\n")
    writeFile.write("}\n")

def WriteThermalDirichletParametersInput(writeFile:TextIO,dirichlet:Iterable[Tuple[str,Dict]]):
    """Write thermal dirichlet parameters input in file
    Prescribed emperature ("temperature") is mandatory. 

    Example:
    import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
    writeFile = AsterCommonWriter.OpenAsterParamFile("myFile.param")
    prescribed_temperature={"temperature":50.0}
    dirichlet = [['X0',prescribed_temperature]]
    WriteThermalDirichletParametersInput(writeFile,dirichlet)
    AsterCommonWriter.CloseAsterParamFile(writeFile)

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    dirichlet : Iterable[Tuple[str,Dict]]
        dirichlet boundary condition description on suitable boundary tags

    Raises
    ------
    Exception
        if "temperature" is missing in the description
    """
    writeFile.write("ther_impo = {'THER_IMPO' : [\n")
    for tagname,params in dirichlet:
        try:
            writeFile.write("          _F(TEMP="+str(params["temperature"])+", GROUP_MA=('"+str(tagname)+"', )),\n")
        except KeyError as e:
            raise Exception("Temperature value is mandatory for Dirichlet condition for boundary "+tagname) from e
    writeFile.write("],\n")
    writeFile.write("}\n")

def WriteTimeParametersInput(writeFile:TextIO,timeParameters:Dict):
    """Write time parameters input in file
    Start time ("start"), end time ("end"), number of time steps ("nsteps") and initial temperature ("starttemperature") are mandatory. 

    Example:
    import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
    writeFile = AsterCommonWriter.OpenAsterParamFile("myFile.param")
    timeParameters={"start":0.0,"end":7200.0,"nsteps":10,"starttemperature":20.0}
    WriteTimeParametersInput(writeFile,timeParameters)
    AsterCommonWriter.CloseAsterParamFile(writeFile)

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    timeParameters : Dict
        time parameters, should contains following keys: "start","end","nsteps","starttemperature"
    """
    errorMessage =  "At least one parameter needed needed amongst start,end,nsteps,starttemperature for the temporal discretization is missing"
    assert {"start","end","nsteps","starttemperature"} <= timeParameters.keys(), errorMessage
    writeFile.write("t_start ="+str(timeParameters["start"])+"\n")
    writeFile.write("t_end ="+str(timeParameters["end"])+"\n")
    writeFile.write("n_steps ="+str(timeParameters["nsteps"])+"\n")
    writeFile.write("temp_0 ="+str(timeParameters["starttemperature"])+"\n")

def WriteTimeParametersInputForEigenValue(writeFile:TextIO,timeParameters:Dict):
    """Write time parameters input for eigenvalue computation in file
    Start time ("start"), end time ("end") and initial temperature ("starttemperature") are mandatory. 

    Example:
    import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
    writeFile = AsterCommonWriter.OpenAsterParamFile("myFile.param")
    timeParameters={"start":0.0,"end":7200.0,"starttemperature":20.0}
    WriteTimeParametersInputForEigenValue(writeFile,timeParameters)
    AsterCommonWriter.CloseAsterParamFile(writeFile)

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    timeParameters : Dict
        time parameters
    """
    errorMessage =  "At least one parameter needed needed amongst start,end,nsteps,starttemperature for the temporal discretization is missing"
    assert {"start","end","starttemperature"} <= timeParameters.keys(), errorMessage
    writeFile.write("t_start ="+str(timeParameters["start"])+"\n")
    writeFile.write("t_end ="+str(timeParameters["end"])+"\n")
    writeFile.write("temp_0 ="+str(timeParameters["starttemperature"])+"\n")


def WriteMaterialParametersInput(writeFile:TextIO,materials:Dict):
    """Write material parameters input in file
    Thermal conductivity ("lambda"), density ("rho") and thermal capacity ("cp") are mandatory for each material. 

    Example:
    import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
    writeFile = AsterCommonWriter.OpenAsterParamFile("myFile.param")
    material= [['AllZones',{"lambda":50.0,"rho":7850.0,"cp":1046.0}]]
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
        writeFile.write('lambda_'+str(numMater)+'='+str(material["lambda"])+'\n')
        rho_cp = material["rho"]*material["cp"]
        writeFile.write('rho_cp_'+str(numMater)+'='+str(rho_cp)+'\n')
        value={}
        value["LAMBDA"]="lambda_"+str(numMater)
        value["RHO_CP"]="rho_cp_"+str(numMater)
        writeFile.write('mater_'+str(numMater)+' = DEFI_MATERIAU(THER=_F('+','.join([ key+"="+value[key] for key in value.keys()])+'))\n')
        if tagname=='AllZones':
            affeMater.append("_F(MATER=("+'mater_'+str(numMater)+", ),TOUT=\'OUI\')")
        else:
            affeMater.append("_F(MATER=("+'mater_'+str(numMater)+", ),GROUP_MA=(\""+str(tagname)+"\",))")
    writeFile.write('materials = {\'AFFE\' :'+str(affeMater).replace("\'_","_").replace(")\'",")").replace("\"_","_").replace(")\"",")").replace("\"\'","\"")+',}\n')
