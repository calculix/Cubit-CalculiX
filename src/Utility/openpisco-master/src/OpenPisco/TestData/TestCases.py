# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import numpy as np

from OpenPisco.Optim.Problems.OptimProblemConcrete import OptimProblemConcrete
from OpenPisco.Optim.Criteria.Criteria import CriteriaFunctionPointer
import  Muscat.ImplicitGeometry.ImplicitGeometryObjects as IGObs
import  Muscat.ImplicitGeometry.ImplicitGeometryOperators as IGOps


class OptimProblemConcreteEuclidean(OptimProblemConcrete):
    def __init__(self,other=None,dataDeepCopy = True):
        super(OptimProblemConcreteEuclidean,self).__init__(other)

    def TakeValuesFrom(self,other):
         self.point[:] = other.point[:]

class TestOptim2DParabol(OptimProblemConcreteEuclidean):
    def __init__(self,other=None,dataDeepCopy = True):
        super(TestOptim2DParabol,self).__init__(other)

        if other is None:
            self.point = np.array([3., 3.])
            o = CriteriaFunctionPointer()
            ## quadratic potential centered in (-2,-2)
            o.SetName(" [-2,-2] ")
            o.f = lambda point: point[1]**2+(point[0]+3)**2
            o.fSensitivity = lambda point: np.array([2*(point[0]+3), 2*point[1]] )
            self.SetObjectiveCriterion(o)

            c0 = CriteriaFunctionPointer()
            c0.SetName("parabol")
            c0.SetUpperBound(0)
            c0.f = lambda point: -point[0]**2+point[1]
            c0.fSensitivity = lambda point:np.array([-2*point[0], 1.])
            self.AddInequalityConstraint(c0)

            c1 = CriteriaFunctionPointer()
            c1.SetName("plane")
            c1.SetUpperBound(0)
            c1.f = lambda point:-point[1]-point[0]-2
            c1.fSensitivity = lambda point: np.array([-1, -1.])
            self.AddInequalityConstraint(c1)

            self.xs = []
            self.ys = []
            self.GUI = False
            self.line = None
        else:
            self.point = np.copy(other.point)
            self.line = other.line
            self.xs = other.xs
            self.ys = other.ys
            self.GUI = other.GUI

    def GetCurrentPoint(self):
        return self.point

    def InitWriter(self):
        import matplotlib.pyplot as plt
        self.GUI = True
        def draw_parabProblem():
            delta = 0.01
            ymin = -1.3
            ymax = 3.2
            xmin = -3.5
            xmax = 5
            x = np.arange(xmin, xmax, delta)
            y = np.arange(ymin, ymax, delta)
            X, Y = np.meshgrid(x, y)
            Z = Y**2 + (X+3)**2
            vals = Z.flatten()
            vals.sort()
            vals = vals[5:-1:int(len(vals)/10)]
            CS = plt.contour(X, Y, Z, vals, colors='#808080',
                             )
            x = np.arange(-np.sqrt(3.19), np.sqrt(3.19), delta)
            y1 = x**2
            plt.plot(x, y1, 'r', label='Constraints')
            x = np.arange(xmin, -ymin-2, delta)
            y3 = -x - 2
            plt.plot(x, y3, 'r')
            plt.plot(-2.5, 0.5, '*g', markersize=15, label='Optimum')
            ax = plt.gca()

            x = np.arange(-np.sqrt(3.19), np.sqrt(3.19), delta)
            x = np.append(x, x[0])
            y = x**2
            ax.fill(x, y, facecolor='gray', alpha=0.5)
            x = np.arange(xmin, -ymin-2, delta)
            y = -x-2
            x = np.append(x, [xmin, xmin])
            y = np.append(y, [y[-1], y[0]])
            ax.fill(x, y, facecolor='gray', alpha=0.5)
            ax.set_aspect('equal')
            ax.autoscale(tight=True)
            plt.xlabel(r'$x_1$', fontsize=20)
            plt.ylabel(r'$x_2$', fontsize=20)
            import matplotlib.lines as lines
            self.line= lines.Line2D([self.point[0]],[self.point[1]])
            ax.add_line(self.line)

        draw_parabProblem()

    def SaveData(self,data,point=None, gradient= None, direction = None ):
        if self.GUI == False: return

        import matplotlib.pyplot as plt

        if point is None:
            x1 = self.point[0]
            x2 = self.point[1]
        else:
            x1 = point[0]
            x2 = point[1]
        self.xs.append(x1)
        self.ys.append(x2)
        self.line.set_data( self.xs, self.ys)
        plt.gcf().canvas.flush_events()
        plt.show(block=False)
        plt.draw()



class TestOptim2D(OptimProblemConcreteEuclidean):
    def __init__(self,other=None,dataDeepCopy = True):
      super(TestOptim2D,self).__init__(other)
      self.point = np.array([2., 2.5])

      if other is not None :
          self.point = np.copy(other.point)
          self.writer = other.writer
      else :
          self.writer = None

          o = CriteriaFunctionPointer()
          ## quadratic potential centered in (-2,-2)
          o.SetName(" [-2,-2] ")
          o.f = lambda point :  (point[0]+3)**2+(point[1]+2)**2.
          o.fSensitivity = lambda point :2*(point+[3, 2] )
          self.SetObjectiveCriterion(o)

          c0 = CriteriaFunctionPointer()
          c0.SetName("NOT_Outside")
          c0.SetUpperBound(0)
          c0.f = lambda point :((point[0])**2+(point[1])**2-4)
          c0.fSensitivity = lambda point : (2*point)
          self.AddInequalityConstraint(c0)

          c1 = CriteriaFunctionPointer()
          c1.SetName("Not y<0")
          c1.SetUpperBound(0)
          c1.f = lambda point :-point[1]
          c1.fSensitivity = lambda point :np.array([0,-1.])
          self.AddInequalityConstraint(c1)

          c2 = CriteriaFunctionPointer()
          c2.SetName("not x>-1")
          c2.f = lambda point :-1-point[0]
          c2.SetUpperBound(0)
          c2.fSensitivity = lambda point :np.array([-1.,0.])
          self.AddInequalityConstraint(c2,False)

    def GetCurrentPoint(self):
        return self.point

    def InitWriter(self):
            import numpy as np
            import matplotlib.pyplot as plt
            import matplotlib.lines as lines
            import matplotlib.transforms as mtransforms
            import matplotlib.text as mtext

            class MyLine(lines.Line2D):
                def __init__(self, *args, **kwargs):
                    # we'll update the position when the line data is set
                    self.text = mtext.Text(0, 0, '')
                    lines.Line2D.__init__(self, *args, **kwargs)
                    self.lineStyles = "-"
                    self.set_marker("o")
                    # we can't access the label attr until *after* the line is
                    # inited
                    self.text.set_text(self.get_label())


                def set_figure(self, figure):
                    self.text.set_figure(figure)
                    lines.Line2D.set_figure(self, figure)

                def set_axes(self, axes):
                    self.text.set_axes(axes)
                    lines.Line2D.set_axes(self, axes)

                def set_transform(self, transform):
                    # 2 pixel offset
                    texttrans = transform + mtransforms.Affine2D().translate(2, 2)
                    self.text.set_transform(texttrans)
                    lines.Line2D.set_transform(self, transform)

                def set_data(self, x, y):
                    if len(x):
                        self.text.set_position((x[-1], y[-1]))

                    lines.Line2D.set_data(self, x, y)

                def draw(self, renderer):
                    # draw my label at the end of the line with 2 pixel offset
                    lines.Line2D.draw(self, renderer)
                    self.text.draw(renderer)

            class MyWriter():
                def __init__(self, *args, **kwargs):
                    self.first_time = True
                    self.x = []
                    self.y = []

                def Write(self,data,x):
                    self.x.append(x[0])
                    self.y.append(x[1])

                    if self.first_time:
                        self.first_time = False
                        self.fig, self.ax = plt.subplots()
                        circle=plt.Circle((0,0),np.sqrt(4), color='b', fill=False)
                        plt.grid()
                        plt.ion()
                        self.line = MyLine(self.x, self.y, mfc='red')#, ms=12, label='Last Point'
                        self.ax.add_line(self.line)
                        self.ax.add_artist(circle)
                        self.ax.set_xlim((-3, 3))
                        self.ax.set_ylim((-3, 3))
                    else:
                        self.line.set_data(self.x,self.y)
                        self.ax.annotate("run : " + str(data['run']) + " : " + str(data['cpt']), xy=(x[0],x[1] ))

                    self.ax.autoscale_view(tight=True)
                    self.ax.relim()
                    self.fig.canvas.flush_events()
                    plt.show(block=False)
                    plt.draw()
            self.writer =  MyWriter()

    def SaveData(self,data,point=None, gradient= None, direction = None ):
        if self.writer is None: return
        if point is None:
            self.writer.Write(data,self.point)
        else:
            self.writer.Write(data,point)

def TestOptimTopoCubeFlexion(fileName,factor=1 ):
    from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory

    from OpenPisco.Optim.Problems.OptimProblemTopoGeneric import OptimProblemTopoGeneric
    from OpenPisco.Optim.Criteria.GeoCriteria import TopoCriteriaVolume
    from OpenPisco.Optim.Criteria.PhyCriteria import TopoCriteriaCompliance

    res = OptimProblemTopoGeneric()
    volCrit = TopoCriteriaVolume()
    comCrit = TopoCriteriaCompliance()

    res.SetObjectiveCriterion(comCrit)
    res.AddInequalityConstraint(volCrit)
    volCrit.SetUpperBound(0.321)

    res.e2 = 0.0001
    from Muscat.MeshTools.ConstantRectilinearMeshTools import CreateConstantRectilinearMesh

    nx = 15*factor; ny = 16*factor; nz = 17*factor
    support = CreateConstantRectilinearMesh(dimensions=[nx,ny,nz], spacing=[1./(nx-1), 1./(ny-1), 1./(nz-1)], origin=[-.5 ,-.5 ,-.5])

    from OpenPisco.PhysicalSolvers.StructuredFEAMecaSolver import StructuredFEAMecaSolver
    PPM = StructuredFEAMecaSolver()
    PPM.SetSupport(support)

    from Muscat.MeshTools.ConstantRectilinearMeshTools import GetMonoIndex

    zdiri = IGObs.ImplicitGeometryPlane(point=[0,0,-0.4],normal=[0,0,1.] )
    zneum = IGObs.ImplicitGeometryCylinder(center1=[0,0,0.4],center2=[0,0,1.5], radius=0.2 )

    zon = IGOps.ImplicitGeometryUnion([zdiri,zneum])

    NeumannTag = support.nodesTags.CreateTag("Neumann")
    DirichletTag = support.nodesTags.CreateTag("Dirichlet")

    for x in range(nx):
        for y in range(ny):
            nn = GetMonoIndex([x,y,nz-1],support.props.get("dimensions"))[0]
            NeumannTag.AddToTag(nn)

            nn = GetMonoIndex([x,y,0],support.props.get("dimensions"))[0]
            DirichletTag.AddToTag(nn)

    from Muscat.MeshContainers.Filters.FilterObjects import NodeFilter
    PPM.internalSolver.AddNeumannCondition( NodeFilter(zone=zneum, nTag="Neumann"),[False, True, False], lambda x :[0,1000,0] )
    PPM.internalSolver.AddDirichletCondition( NodeFilter(zone=zdiri, nTag="Dirichlet"),[True, True, True], lambda x :[0,0,0] )
    PPM.internalSolver.BuildProblem(dofpernode=3)

    comCrit.problem = PPM

    import OpenPisco.Structured.Levelset3D as Levelset3D
    res.point = Levelset3D.LevelSet3D()
    res.point.SetSupport(support)
    res.ResetState()
    res.OnZone = zon

    init = IGOps.ImplicitGeometryUnion([zdiri, zneum, IGObs.ImplicitGeometryCylinder(center1=[0,0,-1],center2=[0,0,1], radius=0.6 )  ])
    res.point.Initialize(init.ApplyVector )
    res.outputEveryLevelsetUpdate= False

    from Muscat.IO.XdmfWriter import XdmfWriter
    tempdir = TemporaryDirectory.GetTempPath()
    writer = XdmfWriter(tempdir+ fileName + '.xmf')
    print(writer.fileName)
    writer.SetTemporal()
    writer.SetBinary()
    writer.SetXmlSizeLimit(0)
    writer.Open()
    res.writer = writer
    return res

def CheckIntegrity(GUI=False):
    TestOptim2DParabol()
    a = TestOptim2D()
    if GUI :
        a.InitWriter()
    TestOptimTopoCubeFlexion('test')
    return "OK"

if __name__ == '__main__':
    print(CheckIntegrity(True))# pragma: no cover
