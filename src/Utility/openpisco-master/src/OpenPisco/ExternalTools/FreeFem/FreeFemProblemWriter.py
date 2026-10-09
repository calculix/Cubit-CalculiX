# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

"""FreefemProblemWriter

Following OpenPisco business logic for external tools such as Freefem++, we provide utilities suitable for the coupling with it.
The responsability of the current module is simply to build a suitable .edp file that can be handled by the Freefem executable.
"""

from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
from OpenPisco.ExternalTools.FreeFem import GetScriptsPath as GetScriptsPath
import OpenPisco.PhysicalSolvers.FieldsNames as FN

class FreeFemWriterBase():
    def __init__(self):
        self.filename  = ""
        self.phyproblem = None
        self.workingDirectory = ""

    def SetFileName(self,filename:str):
        """Set file name

        Parameters
        ----------
        filename : str
            file name
        """
        self.filename = filename

    def SetWorkingDirectory(self):
        """Set Working directory"""
        self.workingDirectory = TemporaryDirectory.GetTempPath()

    def WriteHeader(self):
        """Write .edp Header"""
        import datetime
        today=datetime.date.today()
        self.SetWorkingDirectory()
        writeFile = open(self.workingDirectory+self.filename,"w")
        writeFile.write('// \n')
        writeFile.write('// '+str(self.filename)+'\n')
        writeFile.write('// Created on '+str(today)+'\n')
        writeFile.write('// \n')
        writeFile.write('// Usage FreeFem++'+str(self.filename)+'\n')
        writeFile.write('searchMethod=1; //Necessary for P0-P0 fast interpolation\n')
        writeFile.close()

    def IncludeMacroScripts(self):
        """Include macro scripts (collection of utilities to be used at a higher level)"""
        writeFile = open(self.workingDirectory+self.filename,"a")
        path = GetScriptsPath()
        writeFile.write("include \""+str(path.replace("\\","/"))+"Macros/ModuleFreefem.edp\";\n")
        writeFile.close()

    def DefineSpace(self,order = 1):
        """Define finite element spaces"""
        writeFile = open(self.workingDirectory+self.filename,"a")
        writeFile.write('fespace Xh(Th,[P'+str(order)+'3d,P'+str(order)+'3d,P'+str(order)+'3d]),Vh(Th,P13d),Rh(Th,P03d); // functional spaces and main functions \n')
        writeFile.close()

    def DefineStateVariables(self):
        """Define states variables"""
        writeFile = open(self.workingDirectory+self.filename,"a")
        for i in range(len(self.phyproblem.problems)):
            writeFile.write('macro u'+str(i+1)+'[u'+str(i+1)+'1,u'+str(i+1)+'2,u'+str(i+1)+'3] //\n')
            writeFile.write('macro f'+str(i+1)+'[f'+str(i+1)+'1,f'+str(i+1)+'2,f'+str(i+1)+'3] //\n')
            writeFile.write('Xh u'+str(i+1)+',f'+str(i+1)+';\n')
        writeFile.write('macro v [v1,v2,v3] //\n')
        writeFile.write('Xh v;\n')
        writeFile.write('Vh Xapprox,X,Chi;\n')
        writeFile.close()

    def DefineMaterials(self):
        """Define materials"""
        writeFile = open(self.workingDirectory+self.filename,"a")
        materials = self.phyproblem.materials
        assert len(materials)==1, "Only one material is handled"
        for tagname,material in materials:
            assert tagname=='AllZones',"Sorry material can only be defined on the whole domain for now "
            young = material["young"]
            poisson = material["poisson"]
            writeFile.write('real E='+str(young)+',NU='+str(poisson)+' ;\n')
            writeFile.write('real c1 = E/(1+NU) ;\n')
            writeFile.write('real c2 = NU/(1-2*NU) ;\n')
            writeFile.write('real mu = c1/2. ;\n')
            writeFile.write('real lambda = c1 * c2 ;\n')
        writeFile.close()


    def Levelset(self):
        """Describe space with levelset"""
        writeFile = open(self.workingDirectory+self.filename,"a")
        writeFile.write('{\n')
        writeFile.write('    {\n')
        writeFile.write('       string LSFile = "freefeminput.chi.sol";\n')
        writeFile.write('       ifstream LevelSet(LSFile);\n')
        writeFile.write('       for (int k = 0 ; k<8 ; k++) LevelSet>>header;\n')
        writeFile.write('       for(int k=0;k<Chi.n;k++){LevelSet>>Chi[][k];}\n')
        writeFile.write('    }\n')
        writeFile.write('}\n')
        writeFile.write('X= (Chi<0);\n')
        writeFile.write('Xapprox = X +'+ str(self.phyproblem.eVoid)+'*(1-X) ;\n')
        writeFile.close()


    #TO DO : remove this functions ASAP
    def Loading(self):
        """Define loading"""
        __teststringField=u"""
        string DataFile = "freefeminput.loading.sol";
        Vh G1,G2,G3;
        real[int] xPt(Th.nv) ;
        real[int] yPt(Th.nv) ;
        real[int] zPt(Th.nv) ;
        {
        ifstream Loading(DataFile) ;
                 for (int k = 0 ; k< 8 ; k++) Loading>>header;
                 for (int k = 0 ; k < Th.nv ; k++) Loading >> xPt[k] >> yPt[k]>> zPt[k] ;
                  }
                 G1[] = xPt;
                 G2[] = yPt;
                 G3[] = zPt;
        """
        writeFile = open(self.workingDirectory+self.filename,"a")
        writeFile.write(__teststringField)
        writeFile.close()

    #TO DO : remove this functions ASAP
    def BodyForce(self):
        """Define body forces"""
        __teststringField=u"""
        string DataFile1 = "freefeminput.bodyforce.sol";
        Vh B1,B2,B3;
        real[int] xPt1(Th.nv) ;
        real[int] yPt1(Th.nv) ;
        real[int] zPt1(Th.nv) ;
        {
        ifstream BodyForce(DataFile1) ;
                 for (int k = 0 ; k< 8 ; k++) BodyForce>>header;
                 for (int k = 0 ; k < Th.nv ; k++) BodyForce >> xPt1[k] >> yPt1[k]>> zPt1[k] ;
                  }
                 B1[] = xPt1;
                 B2[] = yPt1;
                 B3[] = zPt1;
        """
        writeFile = open(self.workingDirectory+self.filename,"a")
        writeFile.write(__teststringField)
        writeFile.close()


    def LoadDisplacement(self):
        """Load displacements"""
        __teststringField=u"""
        string DeplFile = "freefeminput.depl.sol";
        macro depl[depl1, depl2, depl3]//
        Xh depl;
        real[int] xPt2(Th.nv) ;
        real[int] yPt2(Th.nv) ;
        real[int] zPt2(Th.nv) ;
        {
        ifstream Disp(DeplFile) ;
                 for (int k = 0 ; k< 8 ; k++) Disp>>header;
                 for (int k = 0 ; k < Th.nv ; k++) Disp >> xPt2[k] >> yPt2[k]>> zPt2[k] ;
                  }
                 depl1[] = xPt2;
                 depl2[] = yPt2;
                 depl3[] = zPt2;
        """
        writeFile = open(self.workingDirectory+self.filename,"a")
        writeFile.write(__teststringField)
        writeFile.close()

    def LoadVectorField(self,fieldname:str):
        """Load vector field

        Parameters
        ----------
        fieldname : str
            field name
        """
        writeFile = open(self.workingDirectory+self.filename,"a")
        writeFile.write('string '+str(fieldname)+'File = "freefeminput.'+str(fieldname)+'.sol"; \n')
        writeFile.write('macro '+str(fieldname)+'['+str(fieldname)+'1,'+str(fieldname)+'2,'+str(fieldname)+'3]//;\n')
        writeFile.write('Xh '+str(fieldname)+';\n')
        writeFile.write('real[int] '+str(fieldname)+'x(Th.nv), '+str(fieldname)+'y(Th.nv), '+str(fieldname)+'z(Th.nv) ;\n')
        writeFile.write('{ ifstream Field('+str(fieldname)+'File); \n')
        writeFile.write('for (int k = 0 ; k< 8 ; k++) Field>>header; \n')
        writeFile.write('for (int k = 0 ; k < Th.nv ; k++) Field >> '+str(fieldname)+'x[k] >> '+str(fieldname)+'y[k]>> '+str(fieldname)+'z[k] ; } \n')
        writeFile.write(str(fieldname)+'1[] = '+str(fieldname)+'x; '+str(fieldname)+'2[]='+str(fieldname)+'y; '+str(fieldname)+'3[]='+str(fieldname)+'z; \n')
        writeFile.close()

    def WriteDisplacement(self):
        """Write displacement"""
        writeFile = open(self.workingDirectory+self.filename,"a")
        for i in range(len(self.phyproblem.problems)):
            writeFile.write('real[int] u'+str(i+1)+'x(Th.nv);\n')
            writeFile.write('real[int] u'+str(i+1)+'y(Th.nv);\n')
            writeFile.write('real[int] u'+str(i+1)+'z(Th.nv);\n')
            writeFile.write('Vh o'+str(i+1)+'1=0, o'+str(i+1)+'2=0, o'+str(i+1)+'3=0;\n')
        for i in range(len(self.phyproblem.problems)):
            writeFile.write('o'+str(i+1)+'1 = u'+str(i+1)+'1;\n')
            writeFile.write('o'+str(i+1)+'2 = u'+str(i+1)+'2;\n')
            writeFile.write('o'+str(i+1)+'3 = u'+str(i+1)+'3;\n')
            writeFile.write('u'+str(i+1)+'x = o'+str(i+1)+'1[];\n')
            writeFile.write('u'+str(i+1)+'y = o'+str(i+1)+'2[];\n')
            writeFile.write('u'+str(i+1)+'z = o'+str(i+1)+'3[];\n')
        writeFile.write('{\n')
        writeFile.write('     {\n')
        writeFile.write('        ofstream Displacement(SolFile);\n')
        writeFile.write('        Displacement.precision(16);\n')
        writeFile.write('        Displacement<<"MeshVersionFormatted 2"<<endl;\n')
        writeFile.write('        Displacement<<""<<endl;\n')
        writeFile.write('        Displacement<<"Dimension 3"<<endl;\n')
        writeFile.write('        Displacement<<""<<endl;\n')
        writeFile.write('        Displacement<<"SolAtVertices"<<endl;\n')
        writeFile.write('        Displacement<<Vh.ndof<<endl;\n')
        solheader = '        Displacement<<" '+str(len(self.phyproblem.problems))
        for i in range(len(self.phyproblem.problems)):
            solheader= solheader+' 2'
        writeFile.write(solheader+'"<<endl; \n')
        writeFile.write('        Displacement<<""<<endl;\n')
        for i in range(len(self.phyproblem.problems)):
            writeFile.write('        for(int k=0;k<Th.nv;k++){Displacement<<u'+str(i+1)+'x(k)<<" "<<u'+str(i+1)+'y(k)<<" "<<u'+str(i+1)+'z(k)<<endl;}\n')
        writeFile.write('        Displacement<<""<<endl;\n')
        writeFile.write('        Displacement<<"End"<<endl;\n')
        writeFile.write('     }\n')
        writeFile.write('}\n')
        writeFile.close()


class FreeFemStaticWriter(FreeFemWriterBase):
    def __init__(self, phyproblem = None ):
        super(FreeFemWriterBase,self).__init__()
        self.filename  = " "
        self.phyproblem = phyproblem

    def IntegralElasticEnergy(self):
        """Define integral elastic energy computation"""
        writeFile = open(self.workingDirectory+self.filename,"a")
        for i in range(len(self.phyproblem.problems)):
            writeFile.write('real J'+str(i+1)+' = 0; \n')
            writeFile.write("      J"+str(i+1)+" = 0.5*(int3d(Th,qfV=qfV1)(Xapprox*(  lambda*div(u"+str(i+1)+")*div(u"+str(i+1)+")+ 2.*mu*( (e(u"+str(i+1)+"))'*e(u"+str(i+1)+"))  ) )  ) ;\n")
        writeFile.write('string intelasticenergyFile= "intelasticenergy.txt";\n')
        writeFile.write('ofstream IntegralElasticEnergy(intelasticenergyFile);\n')
        for i in range(len(self.phyproblem.problems)):
            writeFile.write('IntegralElasticEnergy<<J'+str(i+1)+'<<endl;\n')
        writeFile.close()

    def ElasticEnergy(self):
        """Define elastic energy computation"""
        writeFile = open(self.workingDirectory+self.filename,"a")
        writeFile.write('Rh chiK, etaK;\n')
        writeFile.write('varf varea(unused, chiK) = int3d(Th)(chiK);\n')
        writeFile.write('etaK[] =  varea(0,Rh);\n')
        for i in range(len(self.phyproblem.problems)):
            writeFile.write("varf Energy"+str(i+1)+"(unused,test) = int3d(Th)( 0.5*( lambda*div(u"+str(i+1)+")*div(u"+str(i+1)+")+ 2.*mu*( (e(u"+str(i+1)+"))'*e(u"+str(i+1)+") ))*test*1./volume );\n")
        writeFile.write('Rh Deriv1Global = 0; \n')
        for i in range(len(self.phyproblem.problems)):
            writeFile.write('Deriv1Global[] += Energy'+str(i+1)+'(0,Rh);\n')
        writeFile.write('Vh Deriv1GlobalReg; \n')
        writeFile.write('Average(Deriv1GlobalReg[],Deriv1Global[],etaK[]);\n')
        writeFile.write('string elasticenergyFile= "freefeminput.elasticenergy.sol";\n')
        writeFile.write('WriteScalarField(Deriv1GlobalReg[],Deriv1GlobalReg[].n,elasticenergyFile);\n')
        writeFile.close()

    def Stress(self):
        """Define stress computation"""
        __teststringField=u"""
        string stressFile = "freefeminput.stress.sol";
        real[int] sxx(Th.nv);
        real[int] syy(Th.nv);
        real[int] szz(Th.nv);
        real[int] sxy(Th.nv);
        real[int] sxz(Th.nv);
        real[int] syz(Th.nv);

        Vh s0=0, s1=0, s2=0, s3=0, s4=0, s5=0;
        s0 =  sigma(c1,c2,u1)[0];
        s1 =  sigma(c1,c2,u1)[1];
        s2 =  sigma(c1,c2,u1)[2];
        s3 =  sigma(c1,c2,u1)[3];
        s4 =  sigma(c1,c2,u1)[4];
        s5 =  sigma(c1,c2,u1)[5];
        sxx = s0[];
        syy = s1[];
        szz = s2[];
        sxy = s3[];
        sxz = s4[];
        syz = s5[];
        {
             {
                ofstream Sigma(stressFile);
                Sigma.precision(16);
                Sigma<<"MeshVersionFormatted 2"<<endl;
                Sigma<<""<<endl;
                Sigma<<"Dimension 3"<<endl;
                Sigma<<""<<endl;
                Sigma<<"SolAtVertices"<<endl;
                Sigma<<Vh.ndof<<endl;
                Sigma<<"2 2 2 "<<endl;
                Sigma<<""<<endl;
                for(int k=0;k<Th.nv;k++){Sigma<<sxx(k)<<" "<<syy(k)<<" "<<szz(k)<<" "<<endl;}
                for(int k=0;k<Th.nv;k++){Sigma<<sxy(k)<<" "<<sxz(k)<<" "<<syz(k)<<" "<<endl;}
                Sigma<<""<<endl;
                Sigma<<"End"<<endl;
             }
        }
        """
        writeFile = open(self.workingDirectory+self.filename,"a")
        writeFile.write(__teststringField)
        writeFile.close()

    def VonMises(self):
        """Define von mises computation"""
        writeFile = open(self.workingDirectory+self.filename,"a")
        writeFile.write('Vh vm;\n')
        writeFile.write("vm = sqrt((VV1*sigma(c1,c2,u1))'*sigma(c1,c2,u1)); // von mises stress '\n")
        writeFile.write('string vonmisesFile= "freefeminput.vonmises.sol";\n')
        writeFile.write('WriteScalarField(vm[],vm[].n,vonmisesFile);\n')
        writeFile.close()

    def WriteAuxiliaryScalarGeneration(self):
        """Write auxiliary scalar in file

        Raises
        ------
        NotImplementedError
            Only Integral elastic energy available, raise exception if another one is asked
        """
        scalarsToBeGenerated = [scalar for scalar in self.phyproblem.auxiliaryScalarGeneration.keys() if self.phyproblem.auxiliaryScalarGeneration[scalar]]
        for scalar in scalarsToBeGenerated:
            if scalar == FN.int_potential_energy:
               self.IntegralElasticEnergy()
            else:
                raise NotImplementedError("Only Integral elastic energy auxiliary scalar handled for now")

    def WriteAuxiliaryNodalFieldGeneration(self):
        """Write auxiliary noal field generation"""
        fieldsToBeGenerated=[fieldname for fieldname in self.phyproblem.auxiliaryFieldGeneration.keys() if self.phyproblem.auxiliaryFieldGeneration[fieldname][FN.Nodes]]
        classMethodByField={
                                   FN.potential_energy : self.ElasticEnergy,
                                   FN.von_mises : self.VonMises,
                                   FN.stress : self.Stress
                                }
        for name in fieldsToBeGenerated:
            classMethodByField[name]()


    def ProblemStaticLHS(self,conform:bool=True):
        """Build left-hand side of problem

        Parameters
        ----------
        conform : bool, optional
            conformal configuration or not, by default True (conformal)
        """
        dirichlet = self.phyproblem.dirichlet
        writeFile = open(self.workingDirectory+self.filename,"a")
        if conform:
           writeFile.write('Xapprox = 1;\n')
        else:
           self.Levelset()
        writeFile.write("varf a(u1,v)= int3d(Th)(Xapprox*(lambda*div(u1)*div(v)+ 2.*mu*((e(u1))'*e(v))))//\n")
        self.cpt = 1
        for indexRef,Id in enumerate(dirichlet):
            writeFile.write('    +on('+str(self.cpt+indexRef))
            for i in range(3):
                if Id[1][i] is not None:
                   writeFile.write(',u1'+str(i+1)+'='+str(Id[1][i]))
            writeFile.write(')//\n')
        self.cpt += indexRef+1
        writeFile.write('    ;\n')
        writeFile.write('matrix A = a(Xh, Xh);\n')
        writeFile.write('set(A, solver = sparsesolver); \n')
        writeFile.close()

    def ProblemStaticRHS(self):
        """Build right-hand side of problem"""
        writeFile = open(self.workingDirectory+self.filename,"a")
        neumann = self.phyproblem.neumann
        bodyforce = self.phyproblem.bodyforce
        problems = self.phyproblem.problems
        problemcpt=1

        for indexProblem,ProblemId in enumerate(problems):
            writeFile.write('varf l'+str(problemcpt+indexProblem)+'([unused],v)= // \n')
            if ProblemId in neumann.keys():
               for idl,loadcase in enumerate(neumann[ProblemId]):
                 cptt=self.cpt+idl
                 if isinstance(loadcase[1][0],float):
                    writeFile.write('-int2d(Th,'+str(cptt)+')(('+str(-loadcase[1][0])+')*v1+('+str(-loadcase[1][1])+')*v2+('+str(-loadcase[1][2])+')*v3)//\n')
                 elif isinstance(loadcase[1][0],str):
                    writeFile.write('-int2d(Th,'+str(cptt)+')((-loading1)*v1+(-loading2)*v2+(-loading3)*v3)//\n')

            if ProblemId in bodyforce.keys():
               for loadcase in bodyforce[ProblemId]:
                 if loadcase[0]=='AllZones':
                    if isinstance(loadcase[1][0],float):
                       writeFile.write('-int3d(Th)(('+str(-loadcase[1][0])+')*v1+('+str(-loadcase[1][1])+')*v2+('+str(-loadcase[1][2])+')*v3)//\n')
                    elif isinstance(loadcase[1][0],str):
                       writeFile.write('-int3d(Th)((-bodyforce1)*v1+(-bodyforce2)*v2+(-bodyforce3)*v3)//\n')

            writeFile.write('; \n')
            writeFile.write('f'+str(problemcpt+indexProblem)+'1[] = l'+str(problemcpt+indexProblem)+'(0,Xh);\n')
            writeFile.write('u'+str(problemcpt+indexProblem)+'1[]  = A^-1*f'+str(problemcpt+indexProblem)+'1[]; \n')
        writeFile.close()


    def WriteFreefemProblem(self):
        """Write freefem problem"""
        self.SetFileName("FreefemStatic.edp")
        self.WriteHeader()
        self.IncludeMacroScripts()
        self.DefineSpace()
        self.DefineStateVariables()
        self.DefineMaterials()
        self.ProblemStaticLHS(conform=self.phyproblem.conform)
        self.ProblemStaticRHS()
        self.WriteDisplacement()
        self.WriteAuxiliaryScalarGeneration()
        self.WriteAuxiliaryNodalFieldGeneration()

class FreeFemLSMetricComputationWriter(FreeFemWriterBase):
    def __init__(self, opts:dict ):
        super(FreeFemLSMetricComputationWriter,self).__init__()
        self.filename  = " "
        self.opts=opts

    def LSMetricComputation(self):
        """Compte level set metric"""
        dimension = self.opts["dim"]
        assert dimension in [2,3],"Dim not supported "+str(dimension)
        if self.opts["dim"]==3:
           return self.LSMetricComputation3d()
        elif self.opts["dim"]==2:
           return self.LSMetricComputation2d()

    def LSMetricComputation3d(self):
        """Compte level set metric in 3D"""
        __toprint=u"""
        load "msh3"
        load "mshmet"
        load "medit"
        mesh3 Th;
        string header, MeshFile = "mshmetdata.mesh", LSFile = "mshmetdata.chi.sol", MetricFile = "mshmetdata.sol";
        fespace Vh(Th,P13d);
        Th = readmesh3(MeshFile);
        Vh Chi;

        {
        {
        ifstream LevelSet(LSFile);
        for (int k = 0 ; k<8 ; k++) LevelSet>>header;
        for(int k=0;k<Chi.n;k++){LevelSet>>Chi[][k];}
        }
        }
        Vh h;
        """

        hmin = self.opts["hmin"]
        hmax = self.opts["hmax"]
        err = self.opts.get("err", 1e-2)
        writeFile = open(self.workingDirectory+self.filename,"a")
        writeFile.write(__toprint)
        writeFile.write( \
         "h[] = mshmet(Th, Chi, normalization = 1, levelset = true, nbregul = 0, hmin = "+str(hmin)+ \
         ", hmax = "+str(hmax)+",err = "+str(err)+");\n")
        writeFile.write('savesol(MetricFile, Th, h);\n')

        writeFile.close()

    def LSMetricComputation2d(self):
        """Compte level set metric in 2D"""
        __toprint=u"""
        load "medit"
        load "iovtk"
        mesh Th,ThRemeshed;
        string header, MeshFile = "mshmetdata.mesh", LSFile = "mshmetdata.chi.sol", MetricFile = "mshmetdata.sol", MeshSolutionFile="mshmetsolution.vtk";
        fespace Vh(Th,P1);
        Th = readmesh(MeshFile);
        Vh Chi, m11=0,m22=0,m12=0;

        {
        {
        ifstream LevelSet(LSFile);
        for (int k = 0 ; k<8 ; k++) LevelSet>>header;
        for(int k=0;k<Chi.n;k++){LevelSet>>Chi[][k];}
        }
        }
        """

        hmin = self.opts["hmin"]
        hmax = self.opts["hmax"]
        err = self.opts.get("err", 1e-2)
        writeFile = open(self.workingDirectory+self.filename,"a")
        writeFile.write(__toprint)
        writeFile.write( \
         "ThRemeshed=adaptmesh(Th,Chi,metric=[m11[],m22[],m12[]],hmin = "+str(hmin)+ \
         ", hmax = "+str(hmax)+",err = "+str(err)+");\n")
        writeFile.write('savesol(MetricFile, Th, m11,m22,m12);\n')
        writeFile.write('savevtk(MeshSolutionFile, ThRemeshed);')
        writeFile.close()

    def WriteFreeFemProblem(self):
        """Write freefem problem"""
        self.SetFileName("ComputeLevelSetMetric.edp")
        self.WriteHeader()
        self.LSMetricComputation()


def CheckIntegrity():
    from OpenPisco.PhysicalSolvers.StaticFreeFemSolverMeca import StaticFreeFemSolverMeca

    PPM = StaticFreeFemSolverMeca()
    PPM.conform = True
    material1={"young":20,"poisson":0.3}
    PPM.materials = [['AllZones',material1],]
    PPM.problems = ['idx1']
    PPM.dirichlet= [['Dirichlet1',(0.,0.,0.)]]
    PPM.neumann= {"idx1": [['Neumann1',(0.,-10.,0.)]] }
    PPM.gravity= {"idx1": [['AllZones',(0.,0.,0.)]] }

    writer = FreeFemStaticWriter(phyproblem = PPM)
    writer.WriteFreefemProblem()
    opts={"dim":2,"hmin":1.,"hmax":1.}
    writer = FreeFemLSMetricComputationWriter(opts=opts)
    writer.WriteFreeFemProblem()
    opts={"dim":3,"hmin":1.,"hmax":1.}
    writer = FreeFemLSMetricComputationWriter(opts=opts)
    writer.WriteFreeFemProblem()
    return "OK"

if __name__ == '__main__':
    print(CheckIntegrity()) # pragma: no cover
