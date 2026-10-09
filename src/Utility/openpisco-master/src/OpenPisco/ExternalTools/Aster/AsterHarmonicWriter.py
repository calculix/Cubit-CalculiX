# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

"""AsterHarmonicWriter

This is the dedicated writer file for the harmonic analysis. We refer to the documentation of AsterCommonWriter for a more general perspective.
"""
from typing import TextIO,Dict,Iterable,Tuple

import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter 

def WriteMaterialParametersInput(writeFile:TextIO,materials:Iterable[Tuple[str,Dict]]):
    """Write material parameters input for harmonic analysis in file. 
    Young modulus ("young"), poisson ratio ("poisson") and density ("density") are mandatory for each material. 
    Damping coefficients ("amor_alpha" and "amor_beta") are optional and are equal to 0 by default.
    
    Example:
    import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
    writeFile = AsterCommonWriter.OpenAsterParamFile("myFile.param")
    material1={"young":210000.0,"poisson":0.3,"density":7.85e-09,"amor_alpha":1.0,"amor_beta":0.0}
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
    affeMater=[]
    infoMater={}
    for numMater,(tagname,material) in enumerate(materials):
        infoMater['mater_'+str(numMater)]=[]
        AsterCommonWriter.WriteVariable(writeFile,variableName='young_modulus_'+str(numMater),variableValue=material["young"])
        AsterCommonWriter.WriteVariable(writeFile,variableName='poisson_number_'+str(numMater),variableValue=material["poisson"])
        AsterCommonWriter.WriteVariable(writeFile,variableName='density_'+str(numMater),variableValue=material["density"])
        amorAlpha = material["amor_alpha"] if "amor_alpha" in material.keys() else 0
        AsterCommonWriter.WriteVariable(writeFile,variableName='amor_alpha_'+str(numMater),variableValue=amorAlpha)
        amorBeta = material["amor_beta"] if "amor_beta" in material.keys() else 0
        AsterCommonWriter.WriteVariable(writeFile,variableName='amor_beta_'+str(numMater),variableValue=amorBeta)

        value={}
        value["E"]="young_modulus_"+str(numMater)
        value["NU"]="poisson_number_"+str(numMater)
        value["RHO"]="density_"+str(numMater)
        value["AMOR_ALPHA"]="amor_alpha_"+str(numMater)
        value["AMOR_BETA"]="amor_beta_"+str(numMater)
        infoMater['mater_'+str(numMater)].append(value)

        writeFile.write('mater_'+str(numMater)+' = DEFI_MATERIAU(ELAS=_F('+','.join([ key+"="+value[key] for key in value.keys()])+'))\n')
        if tagname=='AllZones':
            affeMater.append("_F(MATER=("+'mater_'+str(numMater)+", ),TOUT=\'OUI\')")
            infoMater['mater_'+str(numMater)].append("TOUT")
        else:
            affeMater.append("_F(MATER=("+'mater_'+str(numMater)+", ),GROUP_MA=(\""+str(tagname)+"\",))")
            infoMater['mater_'+str(numMater)].append(tagname)
    writeFile.write('materials = {\'AFFE\' :'+str(affeMater).replace("\'_","_").replace(")\'",")").replace("\"_","_").replace(")\"",")").replace("\"\'","\"")+',}\n')
    writeFile.write('info_mater={')
    for material in infoMater.keys():
        writeFile.write(material.replace("\'","")+":[{"+','.join("\""+param+"\""+":"+str(infoMater[material][0][param]) for param in infoMater[material][0].keys())+'},')
        writeFile.write("\""+str(infoMater[material][1]+"\"")+'],\n')
    writeFile.write('}\n')


def WriteModalBasisAnalysisParametersInput(writeFile:TextIO,harmonicParams:Dict):
    """Write modal basis analysis parameters input for harmonic analysis in file. 
    Minimum excitation frequency ("freqExcitMin"), Maximum excitation frequency ("freqExcitMax") and frequency discretization step ("freqExcitStep") are mandatory. 

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    harmonicParams : Dict
        harmonic parameters, various additional options related to the modal basis are described below with examples/explanations
    
    Examples:

    For building a modal basis with the first NMAX_FREQ frequencies
    analysisParams={"freqExcitMin":515.0,"freqExcitMax":525.0,"freqExcitStep":10,"Numbermaxfreq":10}
    
    For building a modal basis where the modes frequency are searched between modalBasisFreqMin and modalBasisFreqMax
    analysisParams={"freqExcitMin":515.0,"freqExcitMax":525.0,"freqExcitStep":10,"modalBasisFreqMin":0,"modalBasisFreqMax":2.1e5}
    
    For building a modal basis based on the excitation frequency, meaning that the modes frequency are searched between 0 and autoModalBasisRangeFactor*modalBasisFreqMax
    analysisParams={"freqExcitMin":515.0,"freqExcitMax":525.0,"freqExcitStep":10,"autoModalBasisRangeFactor":2}

    For building a modal basis with all modes
    analysisParams={"freqExcitMin":515.0,"freqExcitMax":525.0,"freqExcitStep":10,"AllFreq":None}
    
    import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
    writeFile = AsterCommonWriter.OpenAsterParamFile("myFile.param")
    WriteModalBasisAnalysisParametersInput(writeFile,harmonicParams)
    AsterCommonWriter.CloseAsterParamFile(writeFile)

    Raises
    ------
    Exception
        If none of the options for the modal basis are fulfilled
    """
    minExcFreq,maxExcFreq=harmonicParams['freqExcitMin'],harmonicParams['freqExcitMax']
    #Write Excitation frenquencies range
    assert minExcFreq<=maxExcFreq,"freqExcitMax should be greater than freqExcitMin"
    writeFile.write("exc_interval = {'DEBUT' : "+str(minExcFreq)+", ")
    writeFile.write("'INTERVALLE':[_F(JUSQU_A="+str(maxExcFreq)+", ")
    writeFile.write("PAS="+str(harmonicParams['freqExcitStep'])+")]}\n")
    #Write modal basis built instruction
    if "Numbermaxfreq" in harmonicParams.keys():
        writeFile.write("ModalBasis = {'NMAX_FREQ' : "+str(harmonicParams['Numbermaxfreq'])+"}\n")
    elif all(parameter in harmonicParams.keys() for parameter in ["modalBasisFreqMin","modalBasisFreqMax"]):
        minModFreq,maxModFreq=harmonicParams['modalBasisFreqMin'],harmonicParams['modalBasisFreqMax']
        assert minModFreq<=maxModFreq,"modalBasisFreqMax should be greater than modalBasisFreqMin"
        writeFile.write("ModalBasis = {'FREQ' : "+str((minModFreq,maxModFreq))+"}\n")
    elif "autoModalBasisRangeFactor" in harmonicParams.keys():
        minModFreq,maxModFreq=0.0,harmonicParams['autoModalBasisRangeFactor']*harmonicParams['freqExcitMax']
        writeFile.write("ModalBasis = {'FREQ' : "+str((minModFreq,maxModFreq))+"}\n")
    elif "AllFreq" in harmonicParams.keys():
        writeFile.write("ModalBasis = {'ALLFREQ' : 'TOUT'}\n")
    else:
        raise ValueError("No other feature available for now.")

def WritePhysicalBasisAnalysisParametersInput(writeFile:TextIO,harmonicParams:Dict):
    """Write physical basis analysis parameters input in file
    Minimum excitation frequency ("freqExcitMin"), Maximum excitation frequency ("freqExcitMax") and frequency discretization step ("freqExcitStep") are mandatory. 

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    harmonicParams : Dict
        harmonic parameters

    Example:
    import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
    writeFile = AsterCommonWriter.OpenAsterParamFile("myFile.param")
    analysisParams={"freqExcitMin":515.0,"freqExcitMax":525.0,"freqExcitStep":10}
    WritePhysicalBasisAnalysisParametersInput(writeFile,harmonicParams)
    AsterCommonWriter.CloseAsterParamFile(writeFile)

    """
    minExcFreq,maxExcFreq=harmonicParams['freqExcitMin'],harmonicParams['freqExcitMax']
    #Write Excitation frenquencies range
    assert minExcFreq<=maxExcFreq,"freqExcitMax should be greater than freqExcitMin"
    writeFile.write("exc_interval = {'DEBUT' : "+str(minExcFreq)+", ")
    writeFile.write("'INTERVALLE':[_F(JUSQU_A="+str(maxExcFreq)+", ")
    writeFile.write("PAS="+str(harmonicParams['freqExcitStep'])+")]}\n")


def WritePressureParametersInput(writeFile:TextIO,neumann:Dict,problems:Iterable):
    """Write pressure value in file
    
    Example:
    import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
    writeFile = AsterCommonWriter.OpenAsterParamFile("myFile.param")
    neumann = {"idx1": [['ETag1',{"type":"uniform","val":1e+04}]]}
    WritePressureParametersInput(writeFile,neumann=neumann,problems=["idx1"])
    AsterCommonWriter.CloseAsterParamFile(writeFile)

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    neumann : Dict
        pressure description
    problems : Iterable
        loading cases
    """
    writeFile.write("pressures = {\n")
    for problemId in problems:
        if problemId in neumann.keys():
            for idl,_ in enumerate(neumann[problemId]):
                boundary=neumann[problemId][idl][0]
                pressureParams=neumann[problemId][idl][1]
                writeFile.write("'"+str(boundary)+"':{")
                for presParamsKeys,presParamsVal in pressureParams.items():
                    keyInStr="'"+str(presParamsKeys)+"'"
                    if isinstance(presParamsVal, str):
                        valInStr="'"+str(presParamsVal)+"'"
                    else:
                        valInStr=str(presParamsVal)
                    writeFile.write(keyInStr+":"+valInStr+",\n")
                writeFile.write('},\n')
    writeFile.write("}\n")

def WriteHarmonicDisplacementParametersInput(writeFile:TextIO,harmonicDisp:Iterable[Tuple[str,Dict]],group:str="OnNodes"):
    """Write harmonic displacement parameters input

    Example:
    import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
    writeFile = AsterCommonWriter.OpenAsterParamFile("myFile.param")
    harmonicDisp = [['X0',[0,0,0]]]
    WriteHarmonicDisplacementParametersInput(writeFile,harmonicDisp,group)
    AsterCommonWriter.CloseAsterParamFile(writeFile)

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    harmonicDisp : Iterable[Iterable[str,Dict]]
        harmonic displacement description (element/nodal tag followed by parameters value for each tag)
    group : str
        type of group to apply boundary condition on, either "OnNodes" (default) or "OnElements"

    Raises
    ------
    Exception
        Group considered does not exist, only "OnElements" and "OnNodes" allowed
    """
    writeFile.write("harmonicDisp = {'DDL_IMPO' : [\n")
    for harmonicDispId in harmonicDisp:
        diristr=""
        if harmonicDispId[1][0] is not None:
            diristr+="DX="+str(harmonicDispId[1][0])+","
        if harmonicDispId[1][1] is not None:
            diristr+="DY="+str(harmonicDispId[1][1])+","
        if harmonicDispId[1][2] is not None:
            diristr+="DZ="+str(harmonicDispId[1][2])+","
        if group=="OnElements":
            writeFile.write("                   _F("+diristr+" GROUP_MA=('"+str(harmonicDispId[0])+"', )),")
        elif group=="OnNodes":
            writeFile.write("                   _F("+diristr+" GROUP_NO=('"+str(harmonicDispId[0])+"', )),")
        else:
            raise ValueError("This type of group "+group+" does not exist (TODO: check if GROUP_NO really required here)")
    writeFile.write("],\n")
    writeFile.write("}\n")