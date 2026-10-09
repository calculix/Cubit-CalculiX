# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
r"""AnsysMechanicalThermalCommonWriter

Utilities allowing to write an APDL input file .dat for thermal analysis 
The input .dat file read an input mesh, assing material properties and boundary conditions, compute the solution and post-treatment fields 
"""

from typing import TextIO, Iterable


def OpenAnsysDatFile(fileName: str) -> TextIO:
    """Open ANSYS APDL input file."""
    writeFile = open(fileName, "w")
    writeFile.write("/PREP7\n")
    return writeFile


def CloseAnsysDatFile(writeFile: TextIO):
    """Close ANSYS APDL input file."""
    writeFile.close()


def WriteMeshInput(writeFile: TextIO,
                   meshName: str,
                   title: str = "AnsysMechanicalThermalRadiation"):
    """Read mesh and define analysis title."""
    writeFile.write(f"CDREAD,DB,{meshName},cdb\n")
    writeFile.write(f"/TITLE,{title}\n\n")


def WriteElementTypesInput(writeFile: TextIO):
    """Define thermal element types."""
    writeFile.write("ET,1,SOLID291\n")
    writeFile.write("ET,2,SURF252\n\n")


def WriteMaterialParametersInput(writeFile: TextIO,
                                 material: dict,
                                 materialId: int = 1):
    """
    Examples
    --------
    material = {
        "kxx": 11.4,
        "cp": 0.545,
        "density": 8190,
        "emissivity": 0.85,
    }
    """

    if "kxx" in material:
        writeFile.write(f"MP,KXX,{materialId},{material['kxx']}\n")

    if "cp" in material:
        writeFile.write(f"MP,C,{materialId},{material['cp']}\n")

    if "density" in material:
        writeFile.write(f"MP,DENS,{materialId},{material['density']}\n")

    if "emissivity" in material:
        writeFile.write(f"MP,RSVX,{materialId},{material['emissivity']}\n")

    writeFile.write("\n")


def WriteTemperatureBoundaryConditionsInput(writeFile: TextIO, dirichlet: Iterable[dict]):
    """
    Examples
    --------
    dirichlet = [
        {"eTag": "fix", "temperature": 200.0},
    ]
    """

    for bc in dirichlet:
        nTag = bc['eTag'] + "_node"
        writeFile.write(f"CMSEL,S,{nTag}\n")
        writeFile.write(f"D,ALL,TEMP,{bc['temperature']}\n\n")


def WriteRadiationSurfacesInput(writeFile: TextIO, radiation: Iterable[dict]):
    """
    Examples
    --------
    radiation = [
        {
            "eTag": "radiating_surf",
            "emissivity": 0.9,
            "enclosure": 1,
        },
    ]
    """

    writeFile.write("TYPE,2\n\n")

    for surface in radiation:
        nTag = surface['eTag'] + "_node"
        writeFile.write(f"CMSEL,S,{nTag}\n")
        writeFile.write(
            f"SF,ALL,RDSF,{surface['emissivity']},{surface.get('enclosure', 1)}\n"
        )

    writeFile.write("\n")


def WriteRadiationOptionsInput(writeFile: TextIO, ambientTemperature=22.0,):
    """
    Radiation options
    """

    writeFile.write("ALLSEL\n")
    writeFile.write("RSURF\n")

    writeFile.write("TOFFST,273\n")
    writeFile.write(
        f"SPCTEMP,1,{ambientTemperature}\n"
    )

    writeFile.write("stef,5.669e-008\n")
    
def WriteSolverOptionsInput(writeFile: TextIO):

    writeFile.write("AUTOTS,ON\n")
    writeFile.write("NSUB,1,10,1\n")
    writeFile.write("TIME,1.\n")

    writeFile.write(
        "CNVTOL,HEAT,,1.e-004,,1.e-006\n"
    )

    writeFile.write(
        "RADOPT,,0.0001,,1000,0.1,0.1\n"
    )

    writeFile.write(
        "HEMIOPT,10,1.e-006\n"
    )

    writeFile.write(
        "VFOPT,NEW,AnsysMechanicalThermal,vf,,ASCI,1,OFF\n"
    )
    
def WriteSolveInput(writeFile: TextIO):

    writeFile.write("/SOLU\n")
    writeFile.write("ANTYPE,STATIC\n")
    writeFile.write("SOLVE\n")
    writeFile.write("FINISH\n")
    
def WritePostProcessingInput(writeFile: TextIO):

    writeFile.write("/POST1\n")
    writeFile.write("PLNSOL,TEMP\n")
    writeFile.write("FINISH\n")
