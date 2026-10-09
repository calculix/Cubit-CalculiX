# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
import os
import numpy as np

from Muscat.Types import MuscatIndex, MuscatFloat
import Muscat.MeshContainers.Mesh  as UM
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.IO.ReaderBase import ReaderBase
from Muscat.Helpers.Logger import Info

from OpenPisco.MuscatExtentions.MedTools import MEDName, MEDAvailable, MEDComponentsNames

if MEDAvailable:
    import med.medfile as medfileutils
    import med.medmesh as medmeshutils
    import med.medfamily as medfamilyutils
    import med.medfield as medfieldutils
    import med.medlocalization as medlocalizationutils

class NodalFieldNotFoundError(Exception):
    """Med specific Exception"""
    def __init__(self, message):
        message = "Field "+message+" not found  "
        super().__init__(message)

def ReadMed(fileName=None,string=None,out=None):
    reader = MedReader()
    if fileName is not None:
        reader.SetFileName(fileName)
    elif string is not None:
        reader.SetStringToRead(string)
    else:
        raise ValueError("Need a fileName of a string to read")

    result=reader.Read(out=out)
    return result

class MedReader(ReaderBase):
    def __init__(self):
        super(MedReader,self).__init__()
        self.refsAsAField = False
        self.dim = 3
        self.version = -1
        self.meshname = ""
        self.binary = True

    def Read(self,out=None):
        return self.ReadMedBinary(out=out)

    def SetFileName(self,fileName):
        super(MedReader,self).SetFileName(fileName)
        self.SetBinary(True)

    def ReadMedBinary(self, _fileName=None,_string=None,out=None):
        assert out is None,"Unable to read structured mesh in med format "
        self.output = self.InitializeMesh(_fileName,_string)

        fid = medfileutils.MEDfileOpen(self.fileName,medfamilyutils.MED_ACC_RDONLY)
        nmesh = medmeshutils.MEDnMesh(fid)
        Info("Le fichier contient %d maillages"%nmesh)

        if nmesh > 1:
           print("Warning: we are reading only the first mesh ")

        for mesh_index in range(1, nmesh+1):
            Info("Maillage numero : %d"%(mesh_index))
            maa, sdim, dimension, meshtype, desc, dtunit, _, nstep,  repere, axisname, axisunit = medmeshutils.MEDmeshInfo(fid, mesh_index)
            Info("Maillage de nom : |%s| , de dimension : %d , et de type %s"%(maa,dimension,meshtype))
            Info("\t -Dimension de l'espace : %d"%(sdim))
            Info("\t -Description du maillage : |%s|"%(desc))
            Info("\t -Noms des axes : |%s|"%(axisname))
            Info("\t -Unites des axes : |%s|"%(axisunit))
            Info("\t -Type de repere : %s"%(repere))
            Info("\t -Nombre d'etapes de calcul : %d"%(nstep))
            Info("\t -Unite des dates : |%s|"%(dtunit))

        nbNodes,coordinates = self.ReadNodes(fid,maa,dimension)
        self.ReadElements(fid,maa)
        self.ReadFamilies(fid,maa,nbNodes,coordinates)

        medfileutils.MEDfileClose(fid)
        self.output.GenerateManufacturedOriginalIDs()
        self.output.PrepareForOutput()
        return self.output

    def InitializeMesh(self,_fileName,_string):
        if _fileName is not None:
            self.SetFileName(_fileName)

        if _string is not None:
            self.SetStringToRead(_string)

        self.readFormat = 'r'
        self.StartReading()

        res = UM.Mesh()
        return res

    def ReadNodes(self,fid,maa,dimension):
        nbNodes,_,_ = medmeshutils.MEDmeshnEntity(fid,
                                                  maa,
                                                  medfamilyutils.MED_NO_DT,
                                                  medfamilyutils.MED_NO_IT,
                                                  medfamilyutils.MED_NODE,
                                                  medfamilyutils.MED_NO_GEOTYPE,
                                                  medfamilyutils.MED_COORDINATE,
                                                  medfamilyutils.MED_NO_CMODE)  #,MEDCHAR(0), MEDCHAR(0))
        Info("\t -Nombre de noeuds : %d"%(nbNodes))

        dataType = float
        coordinates = medfamilyutils.MEDFLOAT(np.empty((nbNodes*dimension,),dtype = dataType))
        medmeshutils.MEDmeshNodeCoordinateRd(fid,
                                             maa,
                                             medfamilyutils.MED_NO_DT,
                                             medfamilyutils.MED_NO_IT,
                                             medfamilyutils.MED_FULL_INTERLACE,
                                             coordinates)
        self.output.nodes = np.array(coordinates, dtype = dataType).reshape((nbNodes,dimension))
        self.output.originalIDNodes= np.empty((nbNodes,),dtype=MuscatIndex)

        for cpt in range(nbNodes):
            self.output.originalIDNodes[int(cpt)] = int(cpt)
        return nbNodes,coordinates

    def ReadElements(self,fid,maa):
        globalElementCounter = 0
        for elemtype in MEDName.keys():
          Info("Reading elements of type " + str(elemtype) )
          nbElements,_,_ = medmeshutils.MEDmeshnEntity(fid,
                                                       maa,
                                                       medfamilyutils.MED_NO_DT,
                                                       medfamilyutils.MED_NO_IT,
                                                       medfamilyutils.MED_CELL,
                                                       MEDName[elemtype],
                                                       medfamilyutils.MED_CONNECTIVITY,
                                                       medfamilyutils.MED_NODAL)
          if nbElements ==0:
             continue
          elements = self.output.GetElementsOfType(elemtype)
          nufaelmt = medfamilyutils.MEDINT(nbElements)
          elements.Reserve(nbElements)
          elements.originalIds = np.arange(globalElementCounter,globalElementCounter+nbElements)
          elements.cpt = nbElements
          globalElementCounter +=nbElements

          medconnectivity  = [int(x) for x in elements.connectivity.flatten()]
          medconnectivity = medfamilyutils.MEDINT(medconnectivity)
          medmeshutils.MEDmeshElementConnectivityRd(fid,
                                                    maa,
                                                    medfamilyutils.MED_NO_DT,
                                                    medfamilyutils.MED_NO_IT,
                                                    medfamilyutils.MED_CELL,
                                                    MEDName[elemtype],
                                                    medfamilyutils.MED_NODAL,
                                                    medfamilyutils.MED_FULL_INTERLACE,
                                                    medconnectivity )
          elements.connectivity = np.array(medconnectivity,dtype=int).reshape((nbElements,elements.GetNumberOfNodesPerElement()))
          elements.connectivity -= 1
          medmeshutils.MEDmeshEntityFamilyNumberRd(fid,
                                                   maa,
                                                   medfamilyutils.MED_NO_DT,
                                                   medfamilyutils.MED_NO_IT,
                                                   medfamilyutils.MED_CELL,
                                                   MEDName[elemtype],nufaelmt)


    def ReadFamilies(self,fid,maa,nbNodes,coordinates):
        familynumber_to_groupnames={}
        familynumber_to_familynames={}
        nfamilies = medfamilyutils.MEDnFamily(fid,maa)
        Info("Nombre de familles : %d \n"%(nfamilies))
        groupname_to_familynumbers = {}
        for i in range(nfamilies):
            ngroup = medfamilyutils.MEDnFamilyGroup(fid, maa, i+1)
            natt = medfamilyutils.MEDnFamily23Attribute(fid,maa,i+1)
            Info("Famille %d a %d attributs et %d groupes \n"%(i+1,natt,ngroup))
            gro = medfamilyutils.MEDCHAR(medfamilyutils.MED_LNAME_SIZE*ngroup+1)
            nomfam,numfam,gro = medfamilyutils.MEDfamilyInfo(fid,maa,i+1,gro)
            Info("Famille de nom %s et de numero %d : \n"%(nomfam,numfam))
            familynumber_to_familynames[numfam]=nomfam
            namegroups=[]

            for j in range(ngroup):

                namegroup=''.join([gro[i] for i in range(j*medfamilyutils.MED_LNAME_SIZE,j*medfamilyutils.MED_LNAME_SIZE+medfamilyutils.MED_LNAME_SIZE) if gro[i]!='\x00'])
                namegroup=namegroup.strip()
                namegroups.append(namegroup)
                if namegroup not in groupname_to_familynumbers:
                   groupname_to_familynumbers[namegroup] = [numfam]
                else:
                   groupname_to_familynumbers[namegroup].append(numfam)
            l=[namegroup.split() for namegroup in namegroups]
            familynumber_to_groupnames[numfam]=list(set([item for sublist in l for item in sublist]))

        for familynumber,groupnames in familynumber_to_groupnames.items():

            #adding nodal tags
            nufano = medfamilyutils.MEDINT(nbNodes)
            nodesname = medfamilyutils.MEDCHAR(medfamilyutils.MED_LNAME_SIZE*nbNodes)
            medmeshutils.MEDmeshNodeRd(fid,
                                        maa,
                                        medfamilyutils.MED_NO_DT,
                                        medfamilyutils.MED_NO_IT,
                                        medfamilyutils.MED_FULL_INTERLACE,
                                        coordinates,
                                        nodesname,
                                        nufano,
                                        nufano)
            nodefamilynumbers= np.array(nufano,dtype=int)
            for groupname in groupnames:
                for index in np.nonzero(nodefamilynumbers==familynumber):
                    if len(index):
                       self.output.GetNodalTag(str(groupname)).AddToTag(index)

            #adding elements tags
            for elemtype in MEDName.keys():
                nbElements,_,_ = medmeshutils.MEDmeshnEntity(fid,
                                                             maa,
                                                             medfamilyutils.MED_NO_DT,
                                                             medfamilyutils.MED_NO_IT,
                                                             medfamilyutils.MED_CELL,
                                                             MEDName[elemtype],
                                                             medfamilyutils.MED_CONNECTIVITY,
                                                             medfamilyutils.MED_NODAL)
                if nbElements ==0:
                    continue
                elements = self.output.GetElementsOfType(elemtype)
                nelements = elements.GetNumberOfElements()
                elemfamilynumbers = medfamilyutils.MEDINT(nelements)
                medmeshutils.MEDmeshEntityFamilyNumberRd(fid,
                                                         maa,
                                                         medfamilyutils.MED_NO_DT,
                                                         medfamilyutils.MED_NO_IT,
                                                         medfamilyutils.MED_CELL, MEDName[elemtype],elemfamilynumbers)
                elemfamilynumbers= np.array(elemfamilynumbers,dtype=int)
                for groupname in groupnames:
                    for index in np.nonzero(elemfamilynumbers==familynumber):
                        if len(index):
                           elements.GetTag(str(groupname)).AddToTag(index)

    def ReadExtraFields(self,mesh,fileName, fieldNames=None, fieldSupports=None,fieldNumOrders=None):
        if not os.path.exists(fileName):
            raise FileNotFoundError("Med file "+fileName+" not found")

        self.SetFileName(fileName)
        fid = medfileutils.MEDfileOpen(fileName,medfamilyutils.MED_ACC_RDONLY)

        nmesh = medmeshutils.MEDnMesh(fid)
        Info("Le fichier contient %d maillages"%nmesh)

        if nmesh > 1 :
           print("Warning: reading only first mesh, found %d "%nmesh)

        for mesh_index in range(1, nmesh+1):
            Info("Maillage numero : %d"%(mesh_index))
            maa, sdim, mdim, meshtype, desc, dtunit, _, nstep,  repere, axisname, axisunit = medmeshutils.MEDmeshInfo(fid, mesh_index)
            Info("Maillage de nom : |%s| , de dimension : %d , et de type %s"%(maa,mdim,meshtype))
            Info("\t -Dimension de l'espace : %d"%(sdim))
            Info("\t -Description du maillage : |%s|"%(desc))
            Info("\t -Noms des axes : |%s|"%(axisname))
            Info("\t -Unites des axes : |%s|"%(axisunit))
            Info("\t -Type de repere : %s"%(repere))
            Info("\t -Nombre d'etapes de calcul : %d"%(nstep))
            Info("\t -Unite des dates : |%s|"%(dtunit))

            nfield = medfieldutils.MEDnField(fid)
            Info("Le fichier contient %d champs"%nfield)

        if fieldNames is None:
            if fieldSupports is not None:
                raise ValueError("not yet implemented")
            fieldNames=[]
            fieldSupports = []

            for field_index in range(1, nfield+1):
                ncomp = medfieldutils.MEDfieldnComponent(fid,field_index)
                Info( "Champ numero : %d"%(field_index))
                nomcha,_meshname,_local,typcha,comp,unit,_dtunit,_ncstp = medfieldutils.MEDfieldInfo(fid,field_index)
                Info("\t -Nom du champ : |%s| de type %s"%(nomcha,typcha))
                Info("\t -Nombre de composantes : |%d|"%(ncomp))
                Info("\t -Nom des composantes : |%s|"%(comp))
                Info("\t -Unites des composantes : |%s| "%(unit))
                Info("\t -Unites des dates  : |%s| "%(_dtunit))
                Info("\t -Le maillage associe est |%s|"%(_meshname))
                Info("\t -Nombre de sequences de calcul |%d|"%(_ncstp))
                numdt, numit, _ = medfieldutils.MEDfieldComputingStepInfo(fid, nomcha, 1)
                profile_index = 1
                for ElementName, MedName in MEDName.items():
                    medfieldutils.MEDfieldnProfile(fid,nomcha,numdt,numit,medfamilyutils.MED_CELL,MedName)
                    nval,_, _, _, ngauss = medfieldutils.MEDfieldnValueWithProfile(fid,
                                                                     nomcha,
                                                                     numdt,
                                                                     numit,
                                                                     medfamilyutils.MED_CELL,
                                                                     MedName,
                                                                     profile_index,
                                                                     medfamilyutils.MED_COMPACT_STMODE )
                    Info("\t\t -Nombre de valeurs : |%d|"%(nval))
                    if nval>0:
                        fieldNames.append(nomcha)
                        fieldSupports.append(ElementName)

        else:
            if fieldSupports is not None:
                assert len(fieldNames)==len(fieldSupports), "Number of fields and associated supports should be the same"

        for fieldIndex,(fieldName, fieldSupport) in enumerate(zip(fieldNames, fieldSupports)):
            Info("Reading field named "+fieldName+" on "+str(fieldSupport))
            indexofComponentToRead = None
            medfieldName = fieldName

            for compname in MEDComponentsNames:
                if fieldName.endswith(compname):
                   medfieldName = fieldName[0:-3]
                   indexofComponentToRead = MEDComponentsNames.index(compname)

            ncomp = medfieldutils.MEDfieldnComponentByName(fid,medfieldName)

            Info("\t -Nombre de composantes : |%d|"%(ncomp))
            _meshname,_local,typcha,comp,unit,_dtunit,_ncstp = medfieldutils.MEDfieldInfoByName(fid,medfieldName)
            Info("\t -Nom du champ : |%s| de type %s"%(medfieldName,typcha))
            Info("\t -Nom des composantes : |%s|"%(comp))
            Info("\t -Unites des composantes : |%s| "%(unit))
            Info("\t -Unites des dates  : |%s| "%(_dtunit))
            Info("\t -Le maillage associe est |%s|"%(_meshname))
            Info("\t -Nombre de sequences de calcul |%d|"%(_ncstp))

            numdt, numit, _ = medfieldutils.MEDfieldComputingStepInfo(fid, medfieldName, 1)
            Info("\t\t -Nombre de pas : %d, iterations : %d"%(numdt, numit))

            if fieldSupport=="nodes":
                nval = medfieldutils.MEDfieldnValue(fid,
                                                    medfieldName,
                                                    numdt,
                                                    numit,
                                                    medfamilyutils.MED_NODE,
                                                    medfamilyutils.MED_NO_GEOTYPE )
                if nval==0:
                   raise NodalFieldNotFoundError(fieldName)
                value_field_rd=medfamilyutils.MEDFLOAT(nval*ncomp)
                if fieldNumOrders is not None and fieldNumOrders[fieldIndex] is not None:
                    #field at vertices when multiple fields inside a single result
                    numdt=fieldNumOrders[fieldIndex]+1
                    numit=fieldNumOrders[fieldIndex]+1
                medfieldutils.MEDfieldValueRd(fid,
                                              medfieldName,
                                              numdt,
                                              numit,
                                              medfamilyutils.MED_NODE,
                                              medfamilyutils.MED_NO_GEOTYPE ,
                                              medfamilyutils.MED_FULL_INTERLACE,
                                              medfamilyutils.MED_ALL_CONSTITUENT,
                                              value_field_rd)
                Numpy_data = np.zeros((nval,ncomp),dtype=MuscatFloat)
                for i in range(nval):
                    Numpy_data[i,:] = value_field_rd[ncomp*i:ncomp*(i+1)]
                mesh.nodeFields[fieldName] = Numpy_data

            elif str(fieldSupport).endswith("LocalVertices"):
                fieldSupport = fieldSupport[:-len("LocalVertices")]
                MedName = MEDName[fieldSupport]
                elements = mesh.GetElementsOfType(fieldSupport)
                nodesinelement = elements.GetNumberOfNodesPerElement()

                medmeshutils.MEDmeshnEntity(fid,
                                            maa,
                                            medfamilyutils.MED_NO_DT,
                                            medfamilyutils.MED_NO_IT,
                                            medfamilyutils.MED_CELL,
                                            MedName,
                                            medfamilyutils.MED_NUMBER,
                                            medfamilyutils.MED_NODAL  )
                profile_index = 1
                nval,_, _, _, ngauss = medfieldutils.MEDfieldnValueWithProfile(fid,
                                                                                medfieldName,
                                                                                numdt,
                                                                                numit,
                                                                                medfamilyutils.MED_NODE_ELEMENT,
                                                                                MedName,
                                                                                profile_index,
                                                                                medfamilyutils.MED_COMPACT_STMODE)
                if nval==0:
                    raise NodalFieldNotFoundError(fieldName)

                value_field_rd=medfamilyutils.MEDFLOAT(nval*ncomp*nodesinelement)
                medfieldutils.MEDfieldValueRd(fid,
                                                medfieldName,
                                                numdt,
                                                numit,
                                                medfamilyutils.MED_NODE_ELEMENT,
                                                MedName,
                                                medfamilyutils.MED_FULL_INTERLACE,
                                                medfamilyutils.MED_ALL_CONSTITUENT,
                                                value_field_rd)
                Numpy_data = np.array(value_field_rd)
                Numpy_data = Numpy_data.reshape(nval,nodesinelement,ncomp)
                if indexofComponentToRead is not None:
                   Numpy_data = Numpy_data[...,indexofComponentToRead].reshape(nval,nodesinelement,1)
                mesh.elemFields[fieldName] = Numpy_data

            else:
                MedName = MEDName[fieldSupport]
                medmeshutils.MEDmeshnEntity(fid,
                                            maa,
                                            medfamilyutils.MED_NO_DT,
                                            medfamilyutils.MED_NO_IT,
                                            medfamilyutils.MED_CELL,
                                            MedName,
                                            medfamilyutils.MED_NUMBER,
                                            medfamilyutils.MED_NODAL  )
                profile_index = 1
                nval,_, _, _, ngauss = medfieldutils.MEDfieldnValueWithProfile(fid,
                                                                               medfieldName,
                                                                               numdt,
                                                                               numit,
                                                                               medfamilyutils.MED_CELL,
                                                                               MedName,
                                                                               profile_index,
                                                                               medfamilyutils.MED_COMPACT_STMODE )
                if nval==0:
                    raise NodalFieldNotFoundError(fieldName)

                Info("\t\t -Nombre de valeurs : |%d|"%(nval))
                value_field_rd=medfamilyutils.MEDFLOAT(nval*ncomp*ngauss)
                medfieldutils.MEDfieldValueRd(fid,
                                              medfieldName,
                                              numdt,
                                              numit,
                                              medfamilyutils.MED_CELL,
                                              MedName,
                                              medfamilyutils.MED_FULL_INTERLACE,
                                              medfamilyutils.MED_ALL_CONSTITUENT,
                                              value_field_rd)
                Numpy_data = np.array(value_field_rd, dtype=MuscatFloat)
                Numpy_data = Numpy_data.reshape(nval,ngauss,ncomp)
                if indexofComponentToRead is not None:
                   Numpy_data = Numpy_data[...,indexofComponentToRead].reshape(nval,ngauss,1)
                mesh.elemFields[fieldName] = Numpy_data
        return mesh

    def ReadGaussPointsLocalization(self,fileName):
        self.SetFileName(fileName)
        fid = medfileutils.MEDfileOpen(fileName,medfamilyutils.MED_ACC_RDONLY)
        nloc = medlocalizationutils.MEDnLocalization(fid)
        Info("Le fichier contient %d localisations"%nloc)

        for loc_index in range(1,nloc+1):
            locname, typegeo, locsdim, ngauss, _, _, _, sectiongeotype = medlocalizationutils.MEDlocalizationInfo(fid, loc_index)
            Info("\t- Loc. n°%i de nom |%s| de dimension |%d| avec %d pts de GAUSS \n"%(loc_index,locname,locsdim,ngauss))

            medmeshutils.MEDmeshGeotypeName(fid, typegeo)
            locnnodes = medmeshutils.MEDmeshGeotypeParameter(fid,typegeo)
            medmeshutils.MEDmeshGeotypeName(fid, sectiongeotype)
            t1 = locnnodes*locsdim
            t2 = ngauss*locsdim
            t3 = ngauss
            refcoo = medfamilyutils.MEDFLOAT(t1)
            gscoo  = medfamilyutils.MEDFLOAT(t2)
            wg     = medfamilyutils.MEDFLOAT(t3)

            medlocalizationutils.MEDlocalizationRd(fid,
                              locname,
                              medfamilyutils.MED_FULL_INTERLACE, refcoo, gscoo, wg)

            nprefcoo = np.array(refcoo)
            nprefcoo = nprefcoo.reshape(locnnodes,locsdim)
            npgscoo = np.array(gscoo)
            npgscoo = npgscoo.reshape(ngauss,locsdim)
            npwg = np.array(wg)

        return locname, nprefcoo, npgscoo ,npwg

if MEDAvailable :
    from Muscat.IO.IOFactory import RegisterReaderClass
    RegisterReaderClass(".med",MedReader)
    RegisterReaderClass(".rmed",MedReader)

def CheckIntegrity():

    from Muscat.Helpers.CheckTools import SkipTest
    if SkipTest("MED_NO_FAIL"): return "skip"

    from OpenPisco.MuscatExtentions.MedTools import MEDAvailable
    if not MEDAvailable:
        return "skip MED not Available"

    from Muscat.MeshTools.MeshCreationTools import CreateCube

    mesh = CreateCube(dimensions=[3,3,3],spacing=[1./2, 1./2, 1./2],origin=[0, 0, 0],ofTetras=True)

    tris = mesh.GetElementsOfType(ED.Triangle_3)

    sol = np.arange(mesh.GetNumberOfNodes(),dtype=MuscatFloat)[:, None]

    tets = mesh.GetElementsOfType(ED.Tetrahedron_4)
    soltets = np.arange(tets.GetNumberOfElements(),dtype = MuscatFloat)[:, None]

    soltris = np.arange(tris.GetNumberOfElements(),dtype = MuscatFloat)[:, None]
    elemField = {ED.Tetrahedron_4 : soltets,  ED.Triangle_3: soltris }

    displ = np.ones((mesh.GetNumberOfNodes(),3),dtype=MuscatFloat)

    force = np.ones((tris.GetNumberOfElements(),3),dtype=MuscatFloat)
    forceAtTria = {ED.Triangle_3: force }

    from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
    FileName = TemporaryDirectory.GetTempPath()+"TestMedWriter.med"
    from OpenPisco.MuscatExtentions.MedWriter import MedWriter
    writer = MedWriter()
    writer.SetBinary(True)
    writer.Open(FileName)
    writer.Write(mesh,PointFieldsNames=["SolAtVertices","DisplAtNodes"],
                      PointFields=[sol,displ],
                      CellFieldsNames =   ["etherogeneousElemField","ForceAtTria"],
                      CellFields=[elemField,forceAtTria])
    writer.Close()

    print('Reading : ' +str(FileName))
    res = ReadMed(fileName=FileName)
    reader = MedReader()
    res = reader.ReadExtraFields(res,
                                fileName = FileName,
                                fieldNames=["etherogeneousElemField","ForceAtTria","SolAtVertices","DisplAtNodes"],
                                fieldSupports=[ED.Tetrahedron_4,ED.Triangle_3,"nodes","nodes"]
               )

    np.testing.assert_almost_equal(res.nodeFields["SolAtVertices"],sol)
    np.testing.assert_almost_equal(res.nodeFields["DisplAtNodes"],displ)
    np.testing.assert_almost_equal(res.elemFields["ForceAtTria"].flatten(),forceAtTria[ED.Triangle_3].flatten())

    try:
        res = reader.ReadExtraFields(res,
                                fileName = "madeUpName.med",
                                fieldNames=["etherogeneousElemField"],
                                fieldSupports=[ED.Tetrahedron_4,]
               )
    except FileNotFoundError:
        print("File should not be found: ok")

    return 'ok'

if __name__ == '__main__':
    print(CheckIntegrity())
