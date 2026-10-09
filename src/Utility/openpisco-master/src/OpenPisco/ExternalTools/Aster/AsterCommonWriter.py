# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
r"""AsterCommonWriter

Following OpenPisco business logic for external tools such as Code\_Aster, we provide utilities suitable for the coupling with it.
They are used across various physical analysis to produce input files given to Code\_Aster. 

The aim is to be able to build new analysis by:
- relying on a higher level of abstraction module 
- translating high-level concepts in Code\_Aster's own DSL
- avoiding repetitions across analysis.

We describe briefly the business logic and role of the current module. There are 3 type of files:
- .export files: keeps the structure of the differents files used by Code\_Aster to launch a calculation. Each analysis is supposed to be associated to one
- .comm files: command files in python relying on Code\_Aster's own DSL to build the analysis from the beginning to the end (reading the mesh, defining the analysis/boundary condition, postprocessing)
- .py files: typically dedicated writer files, supposed to write on disk information related to the specific usecase at end (material parameter, boundary condition values etc...)
There is also the AsterInterface.py used to build the command line to use Code\_Aster. We refer to the actual file for more details.

This is an example of a simplified AsterDummyAnalysis.export file

P actions make_etude
P memjob 1000000
P memory_limit 1000.0
P mode interactif
P nomjob AsterDummyAnalysis
P version stable
A memjeveux 2048.0
A args -max_base 300000
A tpmax 9000.0
F comm AsterDummyAnalysisparam.in D 1
F comm AsterDummyAnalysis.comm D  1
F mess AsterDummyAnalysis.log R  6
F mmed AsterDummyAnalysisinput.med D  20
F rmed AsterDummyAnalysisoutput.rmed R  80

For more detail about the content and inner working of the .export file, we refer to Code\_Aster documentation. 

This module focuses on the `AsterDummyAnalysisparam.in` file.

This file is typically generated dynamically at runtime by OpenPisco in a readable format and is the first file processed by Code_Aster.

It enables writing more generic and flexible `.comm` files. For example, use-case-specific
parameters such as boundary conditions or tags can be easily defined in this input file.

Because this file is read within the same execution scope, the variables it defines
are directly accessible in the `.comm` file.
"""
from typing import TextIO,Dict,Tuple,Iterable

asterKeywordsByConcept= {
    "dirichlet":{"BY_MULTIPLIER":"dirichlet = {'DDL_IMPO' : [\n",
                 "BY_ELIMINATION":"dirichlet = {'MECA_IMPO' : [\n"
                 },
    "neumann": {"force": {"2D":'FORCE_CONTOUR',"3D":'FORCE_FACE'},
                "pressure" : 'PRES_REP'
               },
    "nodalForce":'FORCE_NODALE',
    "bodyForce":'FORCE_INTERNE'
}

def OpenAsterParamFile(fileName:str)->TextIO:
    """Open Aster dedicated parameter file

    Parameters
    ----------
    fileName : str
        filename

    Returns
    -------
    TextIO
        opened parameter file
    """
    writeFile = open(fileName,"a")
    writeFile.write("DEBUT(PAR_LOT='NON'); \n")
    return writeFile

def CloseAsterParamFile(writeFile:TextIO):
    """Close Aster dedicated parameter file

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    """
    writeFile.write("FIN()")
    writeFile.close()

def WriteVariable(writeFile:TextIO,variableName:str,variableValue:any):
    """Write variable in file

    Example:
    writeFile = OpenAsterParamFile("myFile.param")
    WriteVariable(writeFile=writeFile,variableName="resi_rela",variableValue=0.01)
    CloseAsterParamFile(writeFile)

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    variableName : str
        variable name
    variableValue : any
        variable value
    """
    writeFile.write(variableName+'='+str(variableValue)+'\n')

def WriteBodyForcesParametersInput(writeFile:TextIO,bodyforce:Dict,dimensionality:int,problems:Iterable):
    """Write body forces value in file

    Example:
    writeFile = OpenAsterParamFile("myFile.param")
    bodyforce = {"idx1": [['AllZones',(0.,0.,-1.)]] }
    WriteBodyForcesParametersInput(writeFile,bodyforce=bodyforce,dimensionality=3,problems=["idx1"])
    CloseAsterParamFile(writeFile)

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    bodyforce : Dict
        body force description, number of components should match dimensionality (2 in 2D, 3 in 3D)
    dimensionality : int
        problem dimensionality, correspond to dimension of the mesh (either 2 or 3)
    problems : Iterable
        loading cases
    """
    bodyForceKeyword=asterKeywordsByConcept["bodyForce"]
    WriteVariable(writeFile,"bodyforces","{'"+bodyForceKeyword+"' : [")

    forceDirections=["FX","FY","FZ"]
    indentSpace="                                  "
    for problemId in problems:
        if problemId in bodyforce.keys():
            for idl,loadcase in enumerate(bodyforce[problemId]):
                assert len(loadcase[1])==dimensionality,"body force provided ambiguous: number of components should match mesh dimension"
                if bodyforce[problemId][idl][0]=='AllZones':
                    zoneStr="TOUT='OUI'"
                else:
                    zoneStr="GROUP_MA=('"+"FE"+str(bodyforce[problemId][idl][0])+"', )"
                loadStr=[forceDirections[dimensionAxeId]+"="+str(loadcase[1][dimensionAxeId]) for dimensionAxeId in range(dimensionality)]
                bodyForceStr="_F("+", ".join(loadStr)+", "+zoneStr+"),\n"
                writeFile.write(indentSpace+bodyForceStr)
    writeFile.write("],\n")
    writeFile.write("}\n")

def WriteDirichletParametersInput(writeFile:TextIO,
                                  dirichlet:Iterable[Tuple[str,Iterable]],
                                  dimensionality:int,
                                  method:str="BY_MULTIPLIER",
                                  group:str="OnElements"):
    """Write Dirichlet value in file

    Example
    writeFile = OpenAsterParamFile("myFile.param")
    dirichlet = [['X0',(0.,0.,0.)]]
    WriteDirichletParametersInput(writeFile,dirichlet=dirichlet,dimensionality=3,problems=["idx1"])
    CloseAsterParamFile(writeFile)

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    dirichlet : Iterable[List[str,Iterable]]
        dirichlet boundary condition description on suitable boundary tags
    dimensionality : int
        problem dimensionality, correspond to dimension of the mesh (either 2 or 3)
    method : str, optional
        enforcement of dirichlet condition either with Lagrange multipliers ("BY_MULTIPLIER") or by elimination ("BY_ELIMINATION") or , by default "BY_MULTIPLIER"
    group : str, optional
        where to apply dirichlet boundary condition, either on elements (OnElements) or on nodes (OnNodes), by default "OnElements"

    Raises
    ------
    Exception
        The dirichlet condition can only be enforce with Lagrange multipliers or by elimination
    Exception
        The dirichlet condition can only be enforce on nodes or on elements, any other choice will raise an exception
    """
    try:
        writeFile.write(asterKeywordsByConcept["dirichlet"][method])
    except KeyError as e:
        raise Exception("Aster does know how to enforce the dirichlet condition with the method "+method) from e
    asterLocationByGroup={"OnElements":"GROUP_MA","OnNodes":"GROUP_NO"}
    dofDirections=["DX","DY","DZ"]

    for dirichletId in dirichlet:
        try:
            asterLocation=asterLocationByGroup[group]
        except KeyError as e:
            raise Exception("This type of group "+str(e)+" does not exist (TODO: check if GROUP_NO really required here)") from e
        errorMessage = "Dirichlet boundary condition provided ambiguous: number of components should match mesh dimension"
        assert len(dirichletId[1])==dimensionality,errorMessage

        diristr=""
        for dimensionAxeId in range(dimensionality):
            if dirichletId[1][dimensionAxeId] is not None:
                diristr+=dofDirections[dimensionAxeId]+"="+str(dirichletId[1][dimensionAxeId])+","

        writeFile.write("                   _F("+diristr+" "+asterLocation+"=('"+str(dirichletId[0])+"', )),")

    writeFile.write("],\n")
    writeFile.write("}\n")

def WriteNeumannParametersInput(writeFile:TextIO,neumann:Dict,dimensionality:int,problems:Iterable[str]):
    """Write neumann forces value in file

    Example:
    writeFile = OpenAsterParamFile("myFile.param")
    neumann = {"idx1": [['X1',(0.,0.,-1.)]] }
    WriteNeumannParametersInput(writeFile,neumann=neumann,dimensionality=3,problems=["idx1"])
    CloseAsterParamFile(writeFile)

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    neumann : Dict
        neumann description by load cases, number of components should match dimensionality (2 in 2D, 3 in 3D)
    dimensionality : int
        problem dimensionality, correspond to dimension of the mesh (either 2 or 3)
    problems : Iterable[str]
        loading cases names, names should match the neumann boundary condition load cases
    """
    neumannForceKeyword=asterKeywordsByConcept["neumann"]["force"][str(dimensionality)+"D"]
    WriteVariable(writeFile,"neumann","{'"+neumannForceKeyword+"' : [")
    forceDirections=["FX","FY","FZ"]
    indentSpace="                                  "
    for problemId in problems:
        if problemId in neumann.keys():
            for idl,loadcase in enumerate(neumann[problemId]):
                assert len(loadcase[1])==dimensionality,"Neumann boundary condition provided ambiguous: number of components should match mesh dimension"
                loadStr=[forceDirections[dimensionAxeId]+"="+str(loadcase[1][dimensionAxeId]) for dimensionAxeId in range(dimensionality)]
                neumannStr="_F("+", ".join(loadStr)+", GROUP_MA=('"+str(neumann[problemId][idl][0])+"', )),\n"
                writeFile.write(indentSpace+neumannStr)
    writeFile.write("],\n")
    writeFile.write("}\n")

def WritePressureParametersInput(writeFile:TextIO,neumann:Dict,problems:Iterable):
    """Write pressure value in file
    
    Example:
    writeFile = OpenAsterParamFile("myFile.param")
    neumann = {"idx1": [['X1',1]] }
    WritePressureParametersInput(writeFile,neumann=neumann,dimensionality=3,problems=["idx1"])
    CloseAsterParamFile(writeFile)

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    neumann : Dict
        pressure description by load cases
    problems : Iterable
        loading cases, names should match the neumann boundary condition load cases
    """
    neumannPressureKeyword=asterKeywordsByConcept["neumann"]["pressure"]
    WriteVariable(writeFile,"neumann","{'"+neumannPressureKeyword+"' : [")
    for problemId in problems:
        if problemId in neumann.keys():
            for idl,loadcase in enumerate(neumann[problemId]):
                writeFile.write("                                  _F(PRES="+str(loadcase[1])+", GROUP_MA=('"+str(neumann[problemId][idl][0])+"', )),\n")
    writeFile.write("],\n")
    writeFile.write("}\n")

def WriteNodalForcesParametersInput(writeFile:TextIO,forcenodale:Dict,dimensionality:int,problems:Iterable):
    """Write nodal forces value in file

    Example:
    writeFile = OpenAsterParamFile("myFile.param")
    forcenodale = {"idx1": [['X1',(0.,0.,-1.)]] }
    WriteNodalForcesParametersInput(writeFile,forcenodale=forcenodale,dimensionality=3,problems=["idx1"])
    CloseAsterParamFile(writeFile)

    Parameters
    ----------
    writeFile : TextIO
        currently opened parameter file
    neumann : Dict
        neumann description by load cases, number of components should match dimensionality (2 in 2D, 3 in 3D)
    dimensionality : int
        problem dimensionality, correspond to dimension of the mesh (either 2 or 3)
    problems : Iterable
        loading cases, names should match the neumann boundary condition load cases
    """
    nodalForceKeyword=asterKeywordsByConcept["nodalForce"]
    writeFile.write("forcenodale = {'"+nodalForceKeyword+"' :  [\n")
    forceDirections=["FX","FY","FZ"]
    indentSpace="                                  "
    for problemId in problems:
        if problemId in forcenodale.keys():
            for idl,loadcase in enumerate(forcenodale[problemId]):
                assert len(loadcase[1])==dimensionality,"nodal force provided ambiguous: number of components should match mesh dimension"
                loadStr=[forceDirections[dimensionAxeId]+"="+str(loadcase[1][dimensionAxeId]) for dimensionAxeId in range(dimensionality)]
                nodalForceStr="_F("+", ".join(loadStr)+", GROUP_NO=('"+str(forcenodale[problemId][idl][0])+"', )),\n"
                writeFile.write(indentSpace+nodalForceStr)
    writeFile.write("],\n")
    writeFile.write("}\n")