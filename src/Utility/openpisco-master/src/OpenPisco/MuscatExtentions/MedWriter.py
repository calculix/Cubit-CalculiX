# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import numpy as np

from Muscat.Types import MuscatIndex, MuscatFloat
from Muscat.MeshContainers.Tags import Tag as Tag
from Muscat.IO.WriterBase import WriterBase as WriterBase
import Muscat.MeshContainers.ElementsDescription as ED

from OpenPisco.MuscatExtentions.MedTools import MEDName, MEDLocalizationbyElementName, MEDAvailable
from OpenPisco.MuscatExtentions.PartitionRefining import PartitionRefining

if MEDAvailable :
    import med.medfile as medfileutils
    import med.medmesh as medmeshutils
    import med.medfamily as medfamilyutils
    import med.medfield as medfieldutils
    import med.medprofile as medprofileutils
    import med.medlocalization as medlocalizationutils

def ConvertMedBinaryToASCII(medFile,outputFile):
    from subprocess import run
    run(["mdump3",medFile,"NODALE","FULL_INTERLACE","0",">",outputFile])

class FieldNotSupportedError(Exception):
    """Med specific Exception"""
    def __init__(self, message):
        message = " Field with "+message+" components not supported"
        super().__init__(message)

def WriteMed(filename,mesh,PointFieldsNames=None, binary=True):
    OW = MedWriter()
    OW.SetBinary(binary)
    OW.Open(filename)
    OW.Write(mesh,PointFieldsNames = PointFieldsNames)
    OW.Close()

class MedWriter(WriterBase):
    def __init__(self):
        super(MedWriter,self).__init__()
        self._isBinary = True
        self.meshname = "salome_meca_mesh"
        self.timeunit = "s"

    def __str__(self):
        res  = 'MedWriter : \n'
        res += '   FileName : '+str(self.fileName) + '\n'
        res += '   Binary : ' + ('True' if self.isBinary() else 'False') + "\n"
        return res

    def SetFileName(self,fileName):
        self.fileName = fileName

    def SetSinglePrecission(self,single=True):
        if single:
            self.dataType = np.float32
            self.dataSize = 4
        else:
            self.dataType = np.float64
            self.dataSize = 8

    def Write(self,
              meshObject,
              PointFields=None,
              PointFieldsNames=None,
              CellFieldsNames=None,
              CellFields=None):

        assert self.isBinary(),"ASCII not supported for .med format "
        return self.WriteBINARY(meshObject,
                                PointFieldsNames=PointFieldsNames,
                                PointFields=PointFields,
                                CellFieldsNames= CellFieldsNames,
                                CellFields= CellFields)

    def WriteBINARY(self,meshObject,PointFieldsNames=None,PointFields=None,CellFieldsNames=None, CellFields=None):
        errorMessage = "Med Writer not implemented yet for Constant rectilinear mesh. Please provide an unstructured mesh "
        assert not meshObject.props.get("IsConstantRectilinear"),errorMessage

        fid = self.InitializeMeshFile(meshObject)
        self.WriteMesh(fid,meshObject)
        
        if PointFieldsNames is not None:
            self.WriteNodalFields(fid,meshObject,PointFieldsNames,PointFields)
            
        if CellFields is not None:
            self.WriteCellFields(fid,CellFieldsNames,CellFields)

        medfileutils.MEDfileClose(fid)

    def InitializeMeshFile(self,meshObject):
        fid = medfileutils.MEDfileVersionOpen(self.fileName,
                                              medprofileutils.MED_ACC_CREAT,
                                              medprofileutils.MED_NUM_MAJEUR,
                                              medprofileutils.MED_NUM_MINEUR,
                                              medprofileutils.MED_NUM_RELEASE)
        medfileutils.MEDfileCommentWr(fid, "File created using OpenPisco")

        dimension = meshObject.GetElementsDimensionality()
        meshdim     = dimension
        spacedim    = dimension
        coordNames = ["x","y","z"]
        axisname = "".join([coordNames[axisId].rjust(medprofileutils.MED_SNAME_SIZE) for axisId in range(dimension)])
        axisunit = "".join(["cm".rjust(medprofileutils.MED_SNAME_SIZE)]*dimension)

        medmeshutils.MEDmeshCr(fid,
                               self.meshname,
                               meshdim,
                               spacedim,
                               medprofileutils.MED_UNSTRUCTURED_MESH,
                               self.fileName,
                               self.timeunit,
                               medprofileutils.MED_SORT_DTIT,
                               medprofileutils.MED_CARTESIAN,
                               axisname,
                               axisunit)
        medmeshutils.MEDmeshUniversalNameWr(fid, self.meshname)
        medfamilyutils.MEDfamilyCr(fid,
                                   self.meshname,
                                   medprofileutils.MED_NO_NAME,
                                   0,
                                   0,
                                   medprofileutils.MEDCHAR(0))
        return fid

    def WriteMesh(self,fid,meshObject):
        self.WriteNodes(fid,meshObject)
        self.BuildNodalFamilies(fid,meshObject)
        self.BuildElementsFamilies(fid,meshObject)

    def WriteNodes(self,fid,meshObject):
        coordinates = meshObject.nodes.flatten()
        coordinates = coordinates.astype(MuscatFloat)
        coordinates   = medprofileutils.MEDFLOAT(coordinates)
        medmeshutils.MEDmeshNodeCoordinateWr(fid,
                                            self.meshname,
                                            medprofileutils.MED_NO_DT,
                                            medprofileutils.MED_NO_IT,
                                            medprofileutils.MED_UNDEF_DT,
                                            medprofileutils.MED_FULL_INTERLACE,
                                            meshObject.GetNumberOfNodes(),
                                            coordinates)
        nodalrefs = np.arange(meshObject.GetNumberOfNodes(),dtype=MuscatIndex)
        nodalrefs  = [int(x+1) for x in nodalrefs]
        nodalrefs = medprofileutils.MEDINT(nodalrefs)
        medmeshutils.MEDmeshEntityNumberWr(fid,
                                        self.meshname,
                                        medprofileutils.MED_NO_DT,
                                        medprofileutils.MED_NO_IT,
                                        medprofileutils.MED_NODE,
                                        medprofileutils.MED_NONE,
                                        meshObject.GetNumberOfNodes(),
                                        nodalrefs)

    def BuildNodalFamilies(self,fid,meshObject):
        nodalFamilyNumber = np.zeros(meshObject.GetNumberOfNodes(), dtype=MuscatIndex)
        nodalTagsNames = meshObject.nodesTags.keys()

        groupnumber=1
        groupname_by_groupnumber={}

        if len(nodalTagsNames) > 0:
            for tagname in nodalTagsNames:
                if len(meshObject.nodesTags[tagname]) > 0:
                    groupname_by_groupnumber[groupnumber]=tagname
                    groupnumber+=1

        groupnum_by_groupname= {v: k for k, v in groupname_by_groupnumber.items()}
        nodesTags=meshObject.nodesTags
        cellsByGroupnumber={groupnum_by_groupname[tagname]:np.unique(nodesTags[tagname].GetIds()) for tagname in nodalTagsNames if len(nodesTags[tagname]) > 0}

        PRef=PartitionRefining(cellsByGroupnumber,np.unique(range(meshObject.GetNumberOfNodes())))
        PRef.ExecutePartitioning()
        groupsByFamily,cellsByFamily=PRef.GetGroupsByFamily(),PRef.GetCellsByFamily()
        self.partionForNodalGroups=groupsByFamily

        for family, groups in groupsByFamily.items():
            grouplistinastring = ''.join([groupname_by_groupnumber[x].ljust(medprofileutils.MED_LNAME_SIZE) for x in groups])
            if len(grouplistinastring):
                medfamilyutils.MEDfamilyCr(fid,self.meshname,str('FAMILY_NODAL_'+str(family)), int(family), len(groups), medprofileutils.MEDCHAR(grouplistinastring))
            else:
                cellsByFamily[0]=cellsByFamily[family]
                del cellsByFamily[family]

        for family, familycells in cellsByFamily.items():
            nodalFamilyNumber[familycells] = family
        mednodalFamilyNumber  = [int(x) for x in nodalFamilyNumber]
        mednodalFamilyNumber = medprofileutils.MEDINT(mednodalFamilyNumber)
        medmeshutils.MEDmeshEntityFamilyNumberWr(fid,
                                                 self.meshname,
                                                 medprofileutils.MED_NO_DT,
                                                 medprofileutils.MED_NO_IT,
                                                 medprofileutils.MED_NODE,
                                                 medprofileutils.MED_NONE,
                                                 meshObject.GetNumberOfNodes(),
                                                 medprofileutils.MEDINT(mednodalFamilyNumber))

    def BuildElementsFamilies(self,fid,meshObject):
        elements = meshObject.elements
        cpt_nbrelm=0
        groupnumber=1
        nb_family=[0]
        self.partionForElementGroups=dict()
        for name, elementContainer in elements.items():
            if elementContainer.GetNumberOfElements() == 0:
                continue

            elemtype = MEDName[name]
            nelem = elementContainer.GetNumberOfElements()
            if nelem == 0:
                continue

            elementconnectivity  = [int(x+1) for x in elementContainer.connectivity.flatten()]
            connectivity = medprofileutils.MEDINT(elementconnectivity)
            nelem = int(nelem)

            medmeshutils.MEDmeshElementConnectivityWr(fid,
                                                      self.meshname,
                                                      medprofileutils.MED_NO_DT,
                                                      medprofileutils.MED_NO_IT,
                                                      medprofileutils.MED_UNDEF_DT,
                                                      medprofileutils.MED_CELL,
                                                      elemtype,
                                                      medprofileutils.MED_NODAL,
                                                      medprofileutils.MED_FULL_INTERLACE,
                                                      nelem,
                                                      connectivity)

            elemrefs = np.arange(cpt_nbrelm,cpt_nbrelm+nelem,dtype=MuscatIndex)
            elementnumber  = [int(x+1) for x in elemrefs]
            numelement = medprofileutils.MEDINT(elementnumber)
            medmeshutils.MEDmeshEntityNumberWr(fid,
                                               self.meshname,
                                               medprofileutils.MED_NO_DT,
                                               medprofileutils.MED_NO_IT,
                                               medprofileutils.MED_CELL, elemtype, nelem , numelement )
            cpt_nbrelm+=nelem

            elemFamilyNumber = np.zeros(elementContainer.GetNumberOfElements(), dtype=MuscatIndex)
            groupnumber=1
            groupname_by_groupnumber={}
            if len(elementContainer.tags.keys()) > 0:
                for tagname in elementContainer.tags.keys():
                    if len(elementContainer.tags[tagname]):
                        groupname_by_groupnumber[groupnumber]=tagname
                        groupnumber+=1

            groupnum_by_groupname= {v: k for k, v in groupname_by_groupnumber.items()}
            elementContainer=elementContainer.tags
            cellsByGroupnumber={groupnum_by_groupname[tagname]:np.unique(elementContainer[tagname].GetIds()) for tagname in elementContainer.keys() if len(elementContainer[tagname])}

            PRef=PartitionRefining(cellsByGroupnumber,np.unique(range(nelem)))
            PRef.ExecutePartitioning()
            groupsByFamily,cellsByFamily=PRef.GetGroupsByFamily(),PRef.GetCellsByFamily()
            self.partionForElementGroups[name]=groupsByFamily

            ElementType_FamilyNumber_Start=sum(nb_family)
            for family, groups in groupsByFamily.items():
                grouplistinastring = ''.join([groupname_by_groupnumber[x].ljust(medprofileutils.MED_LNAME_SIZE) for x in groups])
                family_shift=family+ElementType_FamilyNumber_Start
                if len(grouplistinastring):
                    medfamilyutils.MEDfamilyCr(fid,
                                               self.meshname,
                                               str('FAMILY_'+str(-family_shift)+'_'+str(name)),
                                               int(-family_shift),
                                               len(groups),
                                               medprofileutils.MEDCHAR(grouplistinastring))
                else:
                    if 0 not in cellsByFamily.keys():
                        cellsByFamily[0]=[]
                    else:
                        cellsByFamily[0].append(cellsByFamily[family])
                    del cellsByFamily[family]


            for family, familycells in cellsByFamily.items():
                elemFamilyNumber[familycells] = family
            medelemFamilyNumber  = [int(-(x+ElementType_FamilyNumber_Start)) for x in elemFamilyNumber]

            nbfamilyElementwithgroup=len(cellsByFamily.keys())
            if 0 in cellsByFamily.keys():
                nbfamilyElementwithgroup-=1
            nb_family.append(nbfamilyElementwithgroup)
            medelemFamilyNumber=medprofileutils.MEDINT(medelemFamilyNumber)
            medmeshutils.MEDmeshEntityFamilyNumberWr(fid,
                                                     self.meshname,
                                                     medprofileutils.MED_NO_DT,
                                                     medprofileutils.MED_NO_IT,
                                                     medprofileutils.MED_CELL,
                                                     elemtype,
                                                     nelem,
                                                     medelemFamilyNumber)


    def WriteNodalFields(self,fid,meshObject,PointFieldsNames,PointFields):
        for fieldname,field in zip(PointFieldsNames,PointFields):
            meshObject.nodeFields[fieldname]=field
            sol = field
            if len(sol.shape)==1:
                sol=np.reshape(sol,(len(sol),1))

            if len(sol) == 0:
                continue
            elif len(sol.shape) == 1:
                sol = np.reshape(sol,(sol.shape[0],1))

            if sol.shape[1] == 3:
                nbcomponents = sol.shape[1]
                comp = ''.join([x.ljust(medprofileutils.MED_SNAME_SIZE) for x in ('FX','FY','FZ')])
            elif sol.shape[1] == 1 :
                nbcomponents = sol.shape[1]
                comp = " DX "
            elif sol.shape[1] == 6:
                    nbcomponents = sol.shape[1]
                    comp = ''.join([x.ljust(medprofileutils.MED_SNAME_SIZE) for x in ('FX','FY','FZ','MX','MY','MZ')])
            else:
                raise FieldNotSupportedError(str(sol.shape[1]))


            unit = " "
            medfieldutils.MEDfieldCr(fid, fieldname, medprofileutils.MED_FLOAT64, nbcomponents, comp, unit,"",self.meshname)
            field =medprofileutils.MEDFLOAT(meshObject.nodeFields[fieldname].flatten())
            medfieldutils.MEDfieldValueWr(fid,
                                            fieldname,
                                            medprofileutils.MED_NO_DT,
                                            medprofileutils.MED_NO_IT,
                                            0.0,
                                            medprofileutils.MED_NODE,
                                            medprofileutils.MED_NONE,
                                            medprofileutils.MED_FULL_INTERLACE,
                                            medprofileutils.MED_ALL_CONSTITUENT,
                                            meshObject.GetNumberOfNodes(),field)
        meshObject.ComputeGlobalOffset()


    def WriteCellFields(self,fid,CellFieldsNames,CellFields):
        for fieldname,field in zip(CellFieldsNames,CellFields):
            for elemtype, data in field.items():
                if len(data) == 0:
                    continue
                if len(data.shape) ==2:
                    nbentries = data.shape[0]
                    nbcomponents = data.shape[1]
                    if data.shape[1] == 3:
                        comp = ''.join([x.ljust(medprofileutils.MED_SNAME_SIZE) for x in ('FX','FY','FZ')])
                    elif data.shape[1] == 6:
                        comp = ''.join([x.ljust(medprofileutils.MED_SNAME_SIZE) for x in ('SIXX', 'SIYY', 'SIZZ', 'SIXY', 'SIXZ', 'SIYZ')])
                    elif data.shape[1] == 1 :
                        comp = 'DX'
                    else:
                        raise FieldNotSupportedError(str(data.shape[1]))
                    unit = ''.join([''.ljust(medprofileutils.MED_SNAME_SIZE) for _ in range(nbcomponents)])
                    medfieldutils.MEDfieldCr(fid, fieldname, medprofileutils.MED_FLOAT64, nbcomponents, comp, unit,"",self.meshname)
                    field = medprofileutils.MEDFLOAT(data.flatten())
                    MedName = MEDName[elemtype]
                    medfieldutils.MEDfieldValueWr(fid,
                                                    fieldname,
                                                    medprofileutils.MED_NO_DT,
                                                    medprofileutils.MED_NO_IT,
                                                    0.0,
                                                    medprofileutils.MED_CELL,
                                                    MedName,
                                                    medprofileutils.MED_FULL_INTERLACE,
                                                    medprofileutils.MED_ALL_CONSTITUENT,
                                                    nbentries,
                                                    field)
                elif len(data.shape) ==3:
                    nbcomponents = data.shape[2]
                    if data.shape[2] ==6:
                        components = ('SIXX','SIYY','SIZZ', 'SIXY', 'SIXZ', 'SIYZ')
                        componentnames = ''.join([x.ljust(medprofileutils.MED_SNAME_SIZE) for x in components])
                        componentunits = ''.join([''.ljust(medprofileutils.MED_SNAME_SIZE) for _ in range(nbcomponents)])
                    elif data.shape[2] ==4:
                        components = ('SIXX','SIYY','SIZZ', 'SIXY')
                        componentnames = ''.join([x.ljust(medprofileutils.MED_SNAME_SIZE) for x in components])
                        componentunits = ''.join([''.ljust(medprofileutils.MED_SNAME_SIZE) for _ in range(nbcomponents)])
                    elif data.shape[2] == 3:
                        componentnames = ''.join([x.ljust(medprofileutils.MED_SNAME_SIZE) for x in ('FX','FY','FZ')])
                    elif data.shape[2] == 1 :
                        components = ('DX',)
                        componentnames = ''.join([x.ljust(medprofileutils.MED_SNAME_SIZE) for x in components])
                        componentunits = ''.join([''.ljust(medprofileutils.MED_SNAME_SIZE) for _ in range(nbcomponents)])
                    else:
                        raise FieldNotSupportedError(str(data.shape[2]))

                    medfieldutils.MEDfieldCr(fid,fieldname,medprofileutils.MED_FLOAT64,nbcomponents,componentnames,componentunits,self.timeunit,self.meshname)
                    nbIntegrationPoints = data.shape[1]
                    localizationname = self.WriteLocalization(fid,elemtype,nbIntegrationPoints)
                    MedName = MEDName[elemtype]
                    medfieldutils.MEDfieldValueWithProfileWr(fid,
                                                fieldname,medprofileutils.MED_NO_DT,
                                                medprofileutils.MED_NO_IT,
                                                0.0,
                                                medprofileutils.MED_CELL,
                                                MedName,
                                                medprofileutils.MED_UNDEF_STMODE,
                                                medprofileutils.MED_NO_PROFILE,
                                                localizationname,
                                                medprofileutils.MED_FULL_INTERLACE,
                                                medprofileutils.MED_ALL_CONSTITUENT,
                                                data.shape[0],
                                                medprofileutils.MEDFLOAT(data.ravel()) )


    def WriteLocalization(self,fid,CellType,nbIntegrationPoints):
        localization = MEDLocalizationbyElementName[CellType][nbIntegrationPoints]
        name = localization.GetLocalizationName()
        nodescoordinates = localization.GetNodesCoordinates()
        gaussPoints, weights = localization.rule.points, localization.rule.weights

        gaussPoints = list(gaussPoints.ravel())
        weights = list(weights.ravel())
        nbGauss = len(weights)

        medlocalizationutils.MEDlocalizationWr(fid,
                                               name,
                                               MEDName[CellType],
                                               3,
                                               medprofileutils.MEDFLOAT(nodescoordinates),
                                               medprofileutils.MED_FULL_INTERLACE,
                                               nbGauss,
                                               medprofileutils.MEDFLOAT(gaussPoints),
                                               medprofileutils.MEDFLOAT(weights),
                                               medprofileutils.MED_NO_INTERPOLATION,
                                               medprofileutils.MED_NO_MESH_SUPPORT )

        return name

if MEDAvailable :
    from Muscat.IO.IOFactory import RegisterWriterClass
    RegisterWriterClass(".med",MedWriter)
    RegisterWriterClass(".rmed",MedWriter)

def CheckPartitionRefining(partitionElements,partitionNodes):
    check_PartitionElement={ED.Bar_2: {1: [1, 8],
                                     2: [2, 8],
                                     3: [3, 8],
                                     4: [4, 8],
                                     5: [5, 8],
                                     6: [6],
                                     7: [7]},
                            ED.Tetrahedron_4: {1: [1, 5, 6, 7],
                                     2: [2, 5, 6, 7, 8],
                                     3: [3, 5, 6, 8],
                                     4: [4, 5, 7, 8]},
                            ED.Triangle_3: {1: [1, 2, 4, 10], 2: [1, 3, 5, 11],
                                    3: [1, 4, 7, 14], 4: [1, 2, 5, 8],
                                    5: [1, 5, 6, 7, 16],
                                    6: [1, 3, 6, 9],
                                    7: [1, 2, 12],
                                    8: [1, 6, 18],
                                    9: [1, 3, 13],
                                    10: [1, 17],
                                    11: [1, 4, 7, 15]
                                    }
                            }

    assert partitionElements==check_PartitionElement,"Regression observed for element partition refining in med writer"
    check_PartitionNodal={1: [1, 2], 2: [2, 6], 3: [1, 4, 6], 4: [3, 5, 6, 7], 5: [4, 5, 7], 6: [1, 5], 7: [7], 8: [2, 8]}
    assert partitionNodes==check_PartitionNodal,"Regression observed for nodal partition refining in med writer"


def CheckIntegrity():

    from Muscat.Helpers.CheckTools import SkipTest
    if SkipTest("MED_NO_FAIL"): return "skip"

    from OpenPisco.MuscatExtentions.MedTools import MEDAvailable
    if not MEDAvailable:
        return "skip MED not Available"

    import Muscat.MeshContainers.Mesh as UM

    from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
    tempdir = TemporaryDirectory.GetTempPath()
    mymesh = UM.Mesh()
    mymesh.nodes = np.array([[0,0,0],[0,1,0],[1,0,0],[1,1,1],[2,2,2],[1,2,1],[2,1,1],[1,1,2],],dtype=MuscatFloat)
    print(mymesh.nodes)
    mymesh.originalIDNodes = np.arange(8,dtype=MuscatIndex)

    mymesh.nodesTags.CreateTag("Test_0").AddToTag(np.array([0,1,3]))
    mymesh.nodesTags.CreateTag("Test_1").AddToTag(np.array([1,5,7]))
    mymesh.nodesTags.CreateTag("Test_2").AddToTag(np.array([2]))
    mymesh.nodesTags.CreateTag("Test_3").AddToTag(np.array([3,4]))
    mymesh.nodesTags.CreateTag("Test_4").AddToTag(np.array([4,0,2]))
    mymesh.nodesTags.CreateTag("Test_5").AddToTag(np.array([5,2,3]))
    mymesh.nodesTags.CreateTag("Test_6").AddToTag(np.array([6,2,4]))
    mymesh.nodesTags.CreateTag("Test_7").AddToTag(np.array([7]))

    tets = mymesh.GetElementsOfType(ED.Tetrahedron_4)
    tets.AddNewElement([0,1,2,3],0)
    tets.tags.CreateTag("Tetra1").AddToTag(0)
    tets.AddNewElement([4,1,2,3],1)
    tets.tags.CreateTag("Tetra2").AddToTag(1)
    tets.AddNewElement([4,1,3,7],2)
    tets.tags.CreateTag("Tetra3").AddToTag(2)
    tets.AddNewElement([4,1,5,7],3)
    tets.tags.CreateTag("Tetr4").AddToTag(3)
    tets.tags.CreateTag("AllTetra").AddToTag(np.array(range(4)))
    tets.tags.CreateTag("RandomTetra1").AddToTag(np.array([0,1,2]))
    tets.tags.CreateTag("RandomTetra2").AddToTag(np.array([0,1,3]))
    tets.tags.CreateTag("RandomTetra3").AddToTag(np.array([2,1,3]))

    tris = mymesh.GetElementsOfType(ED.Triangle_3)
    tris.AddNewElement([0,1,2],0)
    tris.AddNewElement([1,2,3],1)
    tris.AddNewElement([2,3,0],2)
    tris.AddNewElement([1,3,0],3)
    tris.AddNewElement([4,1,2],4)
    tris.AddNewElement([4,1,3],5)
    tris.AddNewElement([4,1,7],6)
    tris.AddNewElement([1,3,7],7)
    tris.AddNewElement([4,1,5],8)
    tris.AddNewElement([4,7,5],9)
    tris.AddNewElement([1,5,7],10)
    tris.originalIds = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11],dtype=MuscatIndex)
    tris.tags.CreateTag("AllTria").AddToTag(np.arange(11))
    tris.tags.CreateTag("RandomTria1").AddToTag(np.array([0,2,4]))
    tris.tags.CreateTag("RandomTria2").AddToTag(np.array([1,3,5]))
    tris.tags.CreateTag("RandomTria3").AddToTag(np.array([2,6,7]))
    tris.tags.CreateTag("RandomTria4").AddToTag(np.array([8,0,3]))
    tris.tags.CreateTag("RandomTria5").AddToTag(np.array([10,1,8]))
    tris.tags.CreateTag("RandomTria6").AddToTag(np.array([6,7,8]))
    tris.tags.CreateTag("Tria1 ").AddToTag(0)
    tris.tags.CreateTag("Tria2").AddToTag(1)
    tris.tags.CreateTag("Tria3").AddToTag(2)
    tris.tags.CreateTag("Tria4").AddToTag(3)
    tris.tags.CreateTag("Tria5 ").AddToTag(4)
    tris.tags.CreateTag("Tria6").AddToTag(5)
    tris.tags.CreateTag("Tria7").AddToTag(6)
    tris.tags.CreateTag("Tria8").AddToTag(7)
    tris.tags.CreateTag("Tria9").AddToTag(8)
    tris.tags.CreateTag("Tria10").AddToTag(9)
    tris.tags.CreateTag("Tria11").AddToTag(10)
    print(tris)

    bars = mymesh.GetElementsOfType(ED.Bar_2)
    bars.AddNewElement([1,2],0)
    bars.AddNewElement([2,3],1)
    bars.AddNewElement([3,4],2)
    bars.AddNewElement([4,1],3)
    bars.AddNewElement([1,7],4)
    bars.AddNewElement([3,7],5)
    bars.AddNewElement([7,5],6)
    bars.tags.CreateTag("Ridges_0").AddToTag(0)
    bars.tags.CreateTag("Ridges_1").AddToTag(1)
    bars.tags.CreateTag("Ridges_2").AddToTag(2)
    bars.tags.CreateTag("Ridges_3").AddToTag(3)
    bars.tags.CreateTag("Ridges_4").AddToTag(4)
    bars.tags.CreateTag("Ridges_5").AddToTag(5)
    bars.tags.CreateTag("Ridges_6").AddToTag(6)
    bars.tags.CreateTag("RandomRidges3").AddToTag(np.array([0,1,2,3,4]))
    print("My Mesh: ",mymesh)
    mymesh.PrepareForOutput()

    sol = np.arange(mymesh.GetNumberOfNodes(),dtype=MuscatFloat)[:, None]

    tets = mymesh.GetElementsOfType(ED.Tetrahedron_4)
    soltets = np.arange(tets.GetNumberOfElements(),dtype = MuscatFloat)[:, None]

    soltris = np.arange(tris.GetNumberOfElements(),dtype = MuscatFloat)[:, None]
    elemField = {ED.Tetrahedron_4 : soltets,  ED.Triangle_3: soltris }

    displ = np.ones((mymesh.GetNumberOfNodes(),3),dtype=MuscatFloat)

    force = np.ones((tris.GetNumberOfElements(),3),dtype=MuscatFloat)
    forceAtTria = {ED.Triangle_3: force }

    writer = MedWriter()
    writer.SetBinary(True)
    writer.Open(tempdir+"Test_MedWriter.med")
    print(writer)
    writer.Write(mymesh,PointFieldsNames=["SolAtVertices","DisplAtNodes"],
                        PointFields=[sol,displ],
                        CellFieldsNames =   ["etherogeneousElemField","ForceAtTria"],
                        CellFields=[elemField,forceAtTria])
    writer.Close()
    CheckPartitionRefining(partitionElements=writer.partionForElementGroups,partitionNodes=writer.partionForNodalGroups)
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity())# pragma: no cover
