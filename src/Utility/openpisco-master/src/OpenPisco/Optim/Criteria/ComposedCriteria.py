# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
""" 
This module is dedicated to composed criteria, and provide various operations to create a new criterion as a function of existing criteria
"""
from __future__ import annotations

import abc
import re
import math
from typing import Optional,Iterable,Dict,List,Tuple

import sympy as sy
import numpy as np

import Muscat.Helpers.ParserHelper as PH
from Muscat.Types import MuscatFloat
from Muscat.FE.FETools import PrepareFEComputation
from Muscat.FE.Fields.FEField import FEField
from Muscat.MeshTools.MeshFieldOperations import GetFieldTransferOp
from Muscat.MeshContainers.Mesh import Mesh

from OpenPisco.Optim.Criteria.CriteriaFactory import RegisterClass as RegisterCriteriaClass
from OpenPisco.Optim.Criteria.CriteriaFactory import Create
from OpenPisco.Optim.Criteria.Criteria  import CriteriaBase
from OpenPisco.Unstructured.Levelset import LevelSet
from OpenPisco import RETURN_SUCCESS

class ComposedCriteria(CriteriaBase):
    """base class to build a criterion based on existing ones
        Usually, a composed criteria is a function of at least one criterion"""
    def __init__(self, other:Optional[ComposedCriteria]=None):
        """Composed criteria constructor

        Parameters
        ----------
        other : Optional[ComposedCriteria], optional
            another instance of this very criteria to copy attributes from, by default None
        """
        super().__init__(other)
        self.internalCriteria = []
        self.numberOfAllowedSubCriteria = 0
        self.useSubObjectBounds = True

        if other is not None:
            self.numberOfAllowedSubCriteria = other.numberOfAllowedSubCriteria
            self.internalCriteria = [ type(c)(c) for c in other.internalCriteria]
            self.useSubObjectBounds = other.useSubObjectBounds

    def GetInternalCriteria(self,i:int)->CriteriaBase:
        """Retrieve internal criteria within composed criteria based on index

        Parameters
        ----------
        i : int
            index of internal criteria to retrieve

        Returns
        -------
        CriteriaBase
            internal criteria retrieved
        """
        return self.internalCriteria[i]

    def GetNumberOfInternalCriteria(self)->int:
        """Compute actual number of internal criteria

        Returns
        -------
        int
            number of internal criteria
        """
        return len(self.internalCriteria)

    def SetUpperBound(self,val:float):
        """Set upper bound for inequality constraint

        Parameters
        ----------
        val : float
            upper bound value
        """
        self.useSubObjectBounds = False
        super().SetUpperBound(val)

    def GetUpperBound(self)->float:
        """Retrieve upper bound of criterion

        Returns
        -------
        float
            upper bound value
        """
        if self.useSubObjectBounds:
            return self.internalCriteria[0].GetUpperBound()
        else:
            return super().GetUpperBound()

    def UpdateValues(self,point:LevelSet)->int:
        """Update all internal criteria 

        Parameters
        ----------
        point : LevelSet
            levelset object

        Returns
        -------
        int
            whether the updating has succeed
        """
        res = RETURN_SUCCESS
        for criterion in self.internalCriteria:
            res = criterion.UpdateValues(point)
            if res != RETURN_SUCCESS:
                return res

        return res

    def GetNumberOfSolutions(self)->int:
        """Compute overall number of internal criteria solutions

        Returns
        -------
        int
            overall number of solutions
        """
        return np.sum([criterion.GetNumberOfSolutions() for criterion in  self.internalCriteria])

    def GetSolution(self,i:int)->np.ndarray:
        """Retrieve internal criteria solution

        Parameters
        ----------
        i : int
            solution index to retrieve

        Returns
        -------
        np.ndarray
            solution

        """
        for internalCriterion in self.internalCriteria:
            nSols = internalCriterion.GetNumberOfSolutions()
            if i < nSols:
                return internalCriterion.GetSolution(i)
            i -= nSols
        assert i<= 0,"solution not available "

    def GetSolutionName(self,i:int)->str:
        """Retrieve internal criteria solution name

        Parameters
        ----------
        i : int
            solution index

        Returns
        -------
        str
            solution name
        """
        for cpt,internalCriterion in enumerate(self.internalCriteria):
            nSols = internalCriterion.GetNumberOfSolutions()
            if i < nSols:
                return f"C{cpt}_"+internalCriterion.GetSolutionName(i)
            i -= nSols
        assert i<= 0,"solution not available "

    def GetScalarsOutputs(self)->Dict:
        """Retrieve internal criteria scalar outputs

        Returns
        -------
        Dict
            _description_
        """
        res = dict()
        for internalCriterion in self.internalCriteria:
            res.update(internalCriterion.GetScalarsOutputs())
        return res

    @abc.abstractmethod
    def GetValue(self)->float:
        """Compute criteria value, provide implementation of the composition of criteria

        Returns
        -------
        float
            criteria value
        """
        return

    @abc.abstractmethod
    def GetSensitivity(self)->np.ndarray:
        """Compute criteria sensitivity, should be consistent with the criteria value
        One should expect the sensitivity to be the derivative of the composition considered to compute the criteria

        Returns
        -------
        np.ndarray
            criteria sensitivity
        """
        return


def OptToCriteria(ops:Dict, keysToRead:Optional[Iterable]= None,  numberOfExpectedCriterion=1)->List[Tuple[Dict,CriteriaBase]]:
    """Convert options to criteria

    Parameters
    ----------
    ops : Dict
        options
    keysToRead : Optional[Iterable]
        keys to read within criteria options, by default None
    numberOfExpectedCriterion : int, optional
        -1 means at least one, by default 1

    Returns
    -------
    List
        List[Tuple[Dict,CriteriaBase]]

    """
    if keysToRead is None:
       keysToRead = [] 
    
    res = []
    for child in ops["children"] :
        assert child[0] == "Criteria",f"Must be a Criteria object not : {child[0]}"
        data = child[1]
        objtype = data["type"]
        del data["type"]
        keysAndVals = {}
        for key in keysToRead:
            keysAndVals[key] = PH.ReadFloat(data[key])
            del data[key]
        criteria = Create(objtype,data)
        res.append( (keysAndVals,criteria) )
    del ops["children"]

    if numberOfExpectedCriterion < 0:
        assert len(res) >= abs(numberOfExpectedCriterion),f"This criterion needs at least {abs(numberOfExpectedCriterion)} sub criteria"
    else:
        assert len(res) == numberOfExpectedCriterion,f"This criterion needs exactly {numberOfExpectedCriterion} sub criteria"

    return res

def CreateCriteriaOffset(ops:Dict)->CriteriaOffset:
    """Create CriteriaOffset

    Parameters
    ----------
    ops : Dict
        options

    Returns
    -------
    CriteriaOffset
        CriteriaOffset instance endowed with internal criteria
    """
    res = CriteriaOffset()
    subres = OptToCriteria(ops)
    res.internalCriteria.append(subres[0][1])
    PH.ReadProperties(ops, ops, res, typeConversion=True)
    return res

class CriteriaOffset(ComposedCriteria):
    """Composed Criteria to apply an offset and/or the absolute values
    This can be used to simulate a target values with tolerance
       g(x) = |f(x)-offset|
       g'(x) =  g(x) * f'(x)
               -----
               |g(x)|


    this is implemented with a sign function : sign(g(x)) = g(x)/|g(x)|


    Parameters
    ----------
    offset : float
        offset to apply, by default 0.
    absValue : bool
        if the absolute values is applied to the function, by default False
    """
    def __init__(self, other:Optional[CriteriaOffset]=None):
        """Constructor of criteria

        Parameters
        ----------
        other : Optional[CriteriaOffset], optional
             another instance of this very criteria to copy attributes from, by default None
        """
        super().__init__(other)
        self.numberOfAllowedSubCriteria = 1

        if other is None:
            self.SetName("Offset")
            self.offset = float(1.)
            self.absValue = False
        else:
            self.offset = other.offset
            self.absValue = other.absValue

    def GetValue(self)->float:
        """Compute criteria value, provide implementation of the composition of criteria

        Returns
        -------
        float
            criteria value
        """
        res = self.GetInternalCriteria(0).GetValue() - self.offset
        if self.absValue :
            return abs(res)
        else:
            return res

    def GetSensitivity(self)->np.ndarray:
        """Compute criteria sensitivity, based on the derivative of the composition

        Returns
        -------
        np.ndarray
            criteria sensitivity
        """
        if self.absValue :
            s = np.sign(self.GetInternalCriteria(0).GetValue() - self.offset)
            return self.GetInternalCriteria(0).GetSensitivity()*s
        else:
            return self.GetInternalCriteria(0).GetSensitivity()

RegisterCriteriaClass("CriteriaOffset", CriteriaOffset,CreateCriteriaOffset)

###############################################################################################

def CreateCriterionCyclicSymmetry(ops:Dict)->CriterionCyclicSymmetry:
    """Create cyclic symmetry criterion

    Parameters
    ----------
    ops : Dict
        Criteria options

    Returns
    -------
    CriterionCyclicSymmetry
        CriterionCyclicSymmetry instance endowed with internal criteria
    """
    res = CriterionCyclicSymmetry()
    subres = OptToCriteria(ops=ops)
    res.internalCriteria.append(subres[0][1])
    PH.ReadProperties(ops, ops, res, typeConversion=True)
    return res

class CriterionCyclicSymmetry(ComposedCriteria):
    r"""Composed criterion to impose a cyclic symmetry to the given sensitivity 
    This means that the sensitivity field looks the same after being rotated by an angle of ( \frac{2\pi}{nFold} ) radians.

    It can be useful when the optimal geometry is expected to be symmetric but the boundary conditions are not
    The implementation uses the field transfer feature 

    Parameters
    ----------
    nFold : int
        fraction angle
    axis : str
        perform a planar rotation about a fixed axis amongst "x","y","z" 
    """
    def __init__(self, other:Optional[CriterionCyclicSymmetry]=None):
        """Constructor of criteria

        Parameters
        ----------
        other : Optional[CriterionCyclicSymmetry], optional
             another instance of this very criteria to copy attributes from, by default None
        """
        super().__init__(other)
        self.numberOfAllowedSubCriteria = 1

        if other is None:
            self.SetName("CyclicSymmetry")
            self.nFold = int(1)
            self.axis= "x"
        else:
            self.nFold = other.nFold
            self.axis= other.axis

    def GetValue(self)->float:
        """Compute criteria value, provide implementation of the composition of criteria

        Returns
        -------
        float
            criteria value
        """
        return self.GetInternalCriteria(0).GetValue() 
    
    def GetTransferedNodalField(self,inField:np.ndarray,inMesh:Mesh,targetNodes:np.ndarray)->np.ndarray:
        """Compute transfered nodal field from original mesh to target nodes

        Parameters
        ----------
        inField : np.ndarray
            field to be transfered
        inMesh : Mesh
            mesh support associated to the field
        targetNodes : np.ndarray
            target nodes to transfer the fields on

        Returns
        -------
        np.ndarray
            transfered nodal fields
        """
        space, numberings, _, _ = PrepareFEComputation(inMesh,numberOfComponents=1)
        tempfield = FEField("",mesh=inMesh,space=space,numbering=numberings[0])
        op, _ , _ = GetFieldTransferOp(tempfield,targetNodes,method="Interp/Clamp" )
        return op.dot(inField)
    
    def GetRotationMatrixAroundAxis(self,angle:float)->np.ndarray:
        """Compute rotation matrix based on angle and axis

        Parameters
        ----------
        angle : float
            rotation angle

        Returns
        -------
        np.ndarray
            rotation matrix around axis
        """
        axis = self.axis
        assert axis in ["x","y","z"],"Axe %s not covered (x,y,z allowed)"
        if axis=="x":
            return  np.array((
                        [1,0,0],
                        [0.0,math.cos(angle),-math.sin(angle)],
                        [0,math.sin(angle),math.cos(angle)],               
                           ))
        elif axis=="y":
            return np.array((
                        [math.cos(angle),0,math.sin(angle)],
                        [0.0,1.0,0.0],
                        [-math.sin(angle),0,math.cos(angle)],               
                           ))
        elif axis=="z":
            return np.array((
                        [math.cos(angle),-math.sin(angle),0],
                        [math.sin(angle),math.cos(angle),0],
                        [0,0,1],               
                           ))
        else:
            raise ValueError("Axis "+str(axis)+" not recognized")

    def GetSensitivity(self)->np.ndarray:
        """Compute criteria sensitivity, based on the derivative of the composition

        Returns
        -------
        np.ndarray
            criteria sensitivity
        """
        criterion = self.GetInternalCriteria(0)
        res =  criterion.GetSensitivity()
        transferedField = np.zeros( ( criterion.point.support.GetNumberOfNodes(),self.nFold), dtype = MuscatFloat)
        transferedField[:,0] = res
        for i in range(1,self.nFold):
            angle = i*2*np.pi/self.nFold
            rotMat = self.GetRotationMatrixAroundAxis(angle)
            mesh = criterion.point.support.View()
            nodes = mesh.nodes.transpose(1,0).copy()
            mesh.nodes = rotMat.dot(nodes).transpose(1,0).copy()
             
            transferedField[:,i] = self.GetTransferedNodalField(res,mesh,criterion.point.support.nodes)
            
        meanField =  np.mean(transferedField, axis=1)
        return meanField 
         
RegisterCriteriaClass("CriterionCyclicSymmetry", CriterionCyclicSymmetry,CreateCriterionCyclicSymmetry)

###############################################################################################

def CreateCriteriaPower(ops:Dict)->CriteriaPower:
    """Create criteria power instance

    Parameters
    ----------
    ops : Dict
        _description_

    Returns
    -------
    CriteriaPower
        CriteriaPower instance endowed with internal criteria
    """
    res = CriteriaPower()
    subres = OptToCriteria(ops)
    res.internalCriteria.append(subres[0][1])
    PH.ReadProperties(ops, ops, res, typeConversion=True)
    return res

class CriteriaPower(ComposedCriteria):
    """Composed criterion to raise a criterion to a power 

    Parameters
    ----------
    power : float
        value to raise criterion on
    """
    def __init__(self, other:Optional[CriteriaPower]=None):
        """Constructor of criteria

        Parameters
        ----------
        other : Optional[CriteriaPower], optional
             another instance of this very criteria to copy attributes from, by default None
        """
        super().__init__(other)

        if other is None:
            self.power = 1.
            self.SetName("Power")
        else:
            self.power = other.power

    def GetValue(self)->float:
        """Compute criteria value, provide implementation of the composition of criteria

        Returns
        -------
        float
            criteria value
        """
        return abs(self.GetInternalCriteria(0).GetValue())**self.power

    def GetSensitivity(self)->np.ndarray:
        """Compute criteria sensitivity, based on the derivative of the composition

        Returns
        -------
        np.ndarray
            criteria sensitivity
        """
        return self.power*abs(self.GetInternalCriteria(0).GetValue())**(self.power-1)*np.sign(self.GetInternalCriteria(0).GetValue())*self.GetInternalCriteria(0).GetSensitivity()

RegisterCriteriaClass("Power", CriteriaPower,CreateCriteriaPower)

###############################################################################################

def CreateCriteriaFactor(ops:Dict)->CriteriaFactor:
    """Create a criteria factor instance

    Parameters
    ----------
    ops : Dict
        options

    Returns
    -------
    CriteriaFactor
        CriteriaFactor instance endowed with internal criteria
    """
    res = CriteriaFactor()
    subres = OptToCriteria(ops)
    res.internalCriteria.append(subres[0][1])
    PH.ReadProperties(ops, ops, res, typeConversion=True)
    return res

class CriteriaFactor(ComposedCriteria):
    """Composed criterion to multiply a criterion by a factor

    Parameters
    ----------
    factor : float
        criterion factor value
    """
    def __init__(self, other:Optional[CriteriaFactor]=None):
        """Constructor of criteria

        Parameters
        ----------
        other : Optional[CriteriaFactor], optional
             another instance of this very criteria to copy attributes from, by default None
        """
        super().__init__(other)

        self.numberOfAllowedSubCriteria = 1

        if other is None:
            self.factor = float(1.)
        else:
            self.factor = other.factor

    def GetValue(self)->float:
        """Compute criteria value, provide implementation of the composition of criteria

        Returns
        -------
        float
            criteria value
        """
        return self.factor*self.GetInternalCriteria(0).GetValue()

    def GetSensitivity(self)->np.ndarray:
        """Compute criteria sensitivity, based on the derivative of the composition

        Returns
        -------
        np.ndarray
            criteria sensitivity
        """
        return self.factor*self.GetInternalCriteria(0).GetSensitivity()

RegisterCriteriaClass("CriteriaFactor", CriteriaFactor,CreateCriteriaFactor)
###############################################################################################

def CreateCriteriaWeighted(ops:Dict)->CriteriaWeighted:
    """Create criteria weighted instance

    Parameters
    ----------
    ops : Dict
        options

    Returns
    -------
    CriteriaWeighted
        CriteriaWeighted instance endowed with internal criteria
    """
    res = CriteriaWeighted()
    subres = OptToCriteria(ops,["weight"], -1 )
    res.internalCriteria.append(subres[0][1])
    for c_ops, subCrit in subres :
        res.internalCriteria.append(subCrit)
        res.factor[subCrit.GetName()] = PH.ReadFloat(c_ops["weight"])

    PH.ReadProperties(ops, ops, res, typeConversion=True)
    print(res.internalCriteria)
    return res

class CriteriaWeighted(ComposedCriteria):
    """Composed criterion to create a linear combination of criteria with different weight.
    When the criterion is used for the first time, if the option automaticPenalisation is enabled, the weights are automatically calibrated:
     - based on the value of each criteria, if useGradientForAutomaticPenalisation is disabled
     - based on the sensitivity of each criteria, if useGradientForAutomaticPenalisation is enabled 

    Parameters
    ----------
    factor : Dict
        factor value by criterion
    automaticPenalisation : bool
        whether to automatically calibrate the weights, default True
    useGradientForAutomaticPenalisation : bool
        whether to use criteria sensitivity (True) or value (False) for calibrating the weights, default True
    """
    def __init__(self,  other:Optional[CriteriaWeighted]=None):
        """Constructor of criteria

        Parameters
        ----------
        other : Optional[CriteriaWeighted], optional
             another instance of this very criteria to copy attributes from, by default None
        """
        super().__init__(other)

        if other is None:
            self.factor = {}
            self.__name__ = "Weighted"
            self.firstTime = True
            self.automaticPenalisation = True
            self.useGradientForAutomaticPenalisation = True
        else:
            self.factor = other.factor.copy()
            self.firstTime = other.firstTime
            self.automaticPenalisation = other.automaticPenalisation
            self.useGradientForAutomaticPenalisation = other.useGradientForAutomaticPenalisation

    def SetUpperBound(self,val:float):
        """Set criterion upperbound
        Should not be used for this criterion

        Parameters
        ----------
        val : float
            upper bound value

        Raises
        ------
        NotImplementedError
            Constraint not handled for this criterion, always raised
        """
        raise NotImplementedError()

    def GetUpperBound(self):
        """Get criterion upperbound
        Should not be used for this criterion

        Raises
        ------
        NotImplementedError
            Constraint not handled for this criterion, always raised
        """
        raise NotImplementedError()

    def GetValue(self)->float:
        """Compute criteria value, provide implementation of the composition of criteria

        Returns
        -------
        float
            criteria value
        """
        if self.firstTime and self.automaticPenalisation:
            self.firstTime = False
            for criterion in self.internalCriteria:
                oldweight = self.factor[criterion.GetName()]
                if self.useGradientForAutomaticPenalisation:
                    weight = np.linalg.norm(criterion.GetSensitivity())
                else:
                    weight = np.linalg.norm(criterion.GetValue())
                errorMessage = f"Automatic weight for criterion '{criterion.GetName()}' is 0: automatic penalization failed  "
                assert not np.isclose(weight,0.),errorMessage
                self.factor[criterion.GetName()] = oldweight/weight

        res = 0.
        for i in range(self.GetNumberOfInternalCriteria()):
            criterion = self.GetInternalCriteria(i)
            res += self.factor[criterion.GetName()]*criterion.GetValue()
        return res

    def GetSensitivity(self)->np.ndarray:
        """Compute criteria sensitivity, based on the derivative of the composition

        Returns
        -------
        np.ndarray
            criteria sensitivity
        """
        c0 = self.GetInternalCriteria(0)
        res = c0.GetSensitivity()*self.factor[c0.GetName()]
        for i in range(1,self.GetNumberOfInternalCriteria()):
            criterion = self.GetInternalCriteria(i)
            weight = self.factor[criterion.GetName()]
            sensitivity = criterion.GetSensitivity()
            res += weight*sensitivity
        return res

RegisterCriteriaClass("Weighted", CriteriaWeighted,CreateCriteriaWeighted)

###############################################################################################

def CreateCriteriaRatio(ops:Dict)->CriteriaWeighted:
    """Create criteria ration instance

    Parameters
    ----------
    ops : Dict
        options

    Returns
    -------
    CriteriaWeighted
        CriteriaWeighted instance
    """
    res = CriteriaWeighted()
    subres = OptToCriteria(ops=ops,numberOfExpectedCriterion=2 )
    res.internalCriteria.append(subres[0][1])
    for _, subCrit in subres :
        res.internalCriteria.append(subCrit)
    PH.ReadProperties(ops, ops, res, typeConversion=True)
    return res

class CriteriaRatio(ComposedCriteria):
    """Composed criterion to compute the ratio of two criteria
    """
    def __init__(self, other:Optional[CriteriaRatio]=None):
        """Constructor of criteria

        Parameters
        ----------
        other : Optional[CriteriaRatio], optional
             another instance of this very criteria to copy attributes from, by default None
        """
        super().__init__(other)

        self.numberOfAllowedSubCriteria = 2
        if other is None:
            self.SetName("Ratio")

    def SetInternalCriteria(self, numcriteria:CriteriaBase, dencriteria:CriteriaBase):
        """Set internal criterion to compute the ratio

        Parameters
        ----------
        numcriteria : CriteriaBase
            numerator criteria
        dencriteria : CriteriaBase
            denominator criteria
        """
        self.internalCriteria = [numcriteria, dencriteria]

    def GetNumeratorCriteria(self)->CriteriaBase:
        """Retrieve numerator criteria

        Returns
        -------
        CriteriaBase
            numerator criteria
        """
        return self.internalCriteria[0]

    def GetDenominatorCriteria(self)->CriteriaBase:
        """Retrieve denominator criteria

        Returns
        -------
        CriteriaBase
            denominator criteria
        """
        return self.internalCriteria[1]

    def GetValue(self)->float:
        """Compute criteria value, provide implementation of the composition of criteria

        Returns
        -------
        float
            criteria value
        """
        weight = self.GetDenominatorCriteria().GetValue()
        assert not np.isclose(weight,0.),f"Criterion '{self.GetDenominatorCriteria().GetName()}' is 0: Impossible to compute the ratio "
        return self.GetNumeratorCriteria().GetValue()/weight

    def GetSensitivity(self)->np.ndarray:
        """Compute criteria sensitivity, based on the derivative of the composition

        Returns
        -------
        np.ndarray
            criteria sensitivity
        """
        numValue = self.GetNumeratorCriteria().GetValue()
        numSens = self.GetNumeratorCriteria().GetSensitivity()
        denValue = self.GetDenominatorCriteria().GetValue()
        denSens = self.GetDenominatorCriteria().GetSensitivity()
        return  ( numSens*denValue - numValue*denSens)/denValue**2

RegisterCriteriaClass("CriteriaRatio",CriteriaRatio, CreateCriteriaRatio)

###############################################################################################

def CreateCriteriaPolynomialVanishing(ops:Dict)->CriteriaPolynomialVanishing:
    """Create instance of PolynomialVanishing criterion

    Parameters
    ----------
    ops : Dict
        options

    Returns
    -------
    CriteriaPolynomialVanishing
        CriteriaPolynomialVanishing instance
    """
    res = CriteriaPolynomialVanishing()
    subres = OptToCriteria(ops)
    res.internalCriteria.append(subres[0][1])
    for _, subCrit in subres :
        res.internalCriteria.append(subCrit)
    PH.ReadProperties(ops, ops, res, typeConversion=True)
    return res

class CriteriaPolynomialVanishing(ComposedCriteria):
    """
    Allow to generate a polynomial constraint from a given one.

    if J is the internal criterion used as a constraint and B is its upperbound,
    then the polynomial function is B*(J/B-1)*( (J/B-1)**2+1 )+ B
    https://royalsocietypublishing.org/doi/10.1098/rspa.2019.0861

    The beneﬁts of using the polynomial vanishing constraint are twofold. 
    - when J is much greater than B, the constraint is dominated by the cubic term in J, thus speeding up the convergence towards constraint satisfaction. 
    - As the constraint is close to being active (i.e. when J is close to B), the constraint is dominated by the linear term in j, meaning that 
    the polynomial vanishing constraint behaves as a traditional vanishing constraint
    """
    def __init__(self, other:Optional[CriteriaPolynomialVanishing]=None):
        """Constructor of criteria

        Parameters
        ----------
        other : Optional[CriteriaPolynomialVanishing], optional
             another instance of this very criteria to copy attributes from, by default None
        """
        super().__init__(other)
        self.numberOfAllowedSubCriteria = 1

        if other is None:
            self.SetName("PolynomialVanishing")

        self.useSubObjectBounds = True
        self.slack = 0.

    def GetSlack(self)->float:
        """Compute internal criterion slack value

        Returns
        -------
        float
            internal criterion slack value
        """
        return self.GetInternalCriteria(0).GetValue()/self.GetUpperBound() - 1.

    def GetValue(self)->float:
        """Compute criteria value, provide implementation of the composition of criteria

        Returns
        -------
        float
            criteria value
        """
        slack = self.GetSlack()
        return self.GetUpperBound()*slack*( slack**2 + 1) + self.GetUpperBound()

    def GetSensitivity(self)->np.ndarray:
        """Compute criteria sensitivity, based on the derivative of the composition

        Returns
        -------
        np.ndarray
            criteria sensitivity
        """
        slack = self.GetSlack()
        return ( 3*slack**2 +1 )*self.internalCriteria.GetSensitivity()

RegisterCriteriaClass("CriteriaPolynomialVanishing",CriteriaPolynomialVanishing,CreateCriteriaPolynomialVanishing)

class SymbolicComposedCriteria(CriteriaBase):
    """Composed criterion to compute a criterion based on a symbolic functional description

    Parameters
    ----------
    criteriaByName : Dict
        mapping between name and criteria instances, theses names are the one used in the criteria expression

    criteriaExpression : str
        symbolic functional description (must be compatible with sympy logic)
    """
    def __init__(self, other:Optional[SymbolicComposedCriteria]=None):
        """Constructor of criteria

        Parameters
        ----------
        other : Optional[SymbolicComposedCriteria], optional
             another instance of this very criteria to copy attributes from, by default None
        """
        super(SymbolicComposedCriteria, self).__init__(other)

        self.omegaAsSymbol = "(omega)"
        if other is None:
            self.SetName("SymbolicComposed")
            self.compoundSymbExpression = dict()
            self.symbolicCritExpr=None
            self.symbolicSensitivExpr=None
            self.symbolicSensitivFunctor=None
        else:
            self.compoundSymbExpression = other.symbExpression
            self.symbolicCritExpr=other.symbolicCritExpr
            self.symbolicSensitivExpr=other.symbolicSensitivExpr
            self.symbolicSensitivFunctor=other.symbolicSensitivFunctor

    def SetComposedSymbExpression(self,criteriaExpression:str,criteriaByName:Dict):
        """Set composed sumbolic expression

        Parameters
        ----------
        criteriaExpression : str
            symbolic functional description
        criteriaByName : Dict
            mapping between name and criteria instances
        """
        self.compoundSymbExpression={"expr":criteriaExpression,"criteria":criteriaByName}
        self.Eval()

    def Eval(self):
        """Evaluation of criteria symbolic value and symbolic sensitivity"""
        expression=self.compoundSymbExpression.get("expr")
        criteria=self.compoundSymbExpression.get("criteria")
        splitted_expr=re.split(r'(\W)', expression)
        new_expr=[None]*len(splitted_expr)

        for termId,term in enumerate(splitted_expr):
            if term in criteria.keys():
                new_expr[termId]=term+self.omegaAsSymbol
            else:
                new_expr[termId]=term
        self.symbolicCritExpr=sy.sympify(''.join(new_expr))

        symbolicSensitivExpr=sy.Derivative(self.symbolicCritExpr).doit()
        for symbolName in criteria.keys():
            symbolicSensitivExpr=symbolicSensitivExpr.subs(sy.sympify(symbolName+self.omegaAsSymbol),sy.sympify(symbolName))
            symbolicSensitivExpr=symbolicSensitivExpr.subs(sy.Derivative(sy.sympify(symbolName), sy.sympify("omega")), sy.sympify("Derivative"+symbolName))
        self.symbolicSensitivExpr=symbolicSensitivExpr

        criteriaArguments=[(sy.sympify(symbolName),sy.sympify("Derivative"+symbolName)) for symbolName in criteria.keys()]
        criteriaArguments=[item for sublist in criteriaArguments for item in sublist]
        self.symbolicSensitivFunctor = sy.lambdify(criteriaArguments, symbolicSensitivExpr, 'numpy')

    def UpdateValues(self,point:LevelSet)->int:
        """Update all internal criteria 

        Parameters
        ----------
        point : LevelSet
            levelset object

        Returns
        -------
        int
            whether the updating has succeed
        """
        self.point = point
        for criterion in self.compoundSymbExpression.get("criteria").values():
            res = criterion.UpdateValues(point)
            if res != RETURN_SUCCESS:
                return res
        return RETURN_SUCCESS

    def GetNumberOfSolutions(self)->int:
        """Compute overall number of internal criteria solutions

        Returns
        -------
        int
            overall number of solutions
        """
        return np.sum([criterion.GetNumberOfSolutions() for criterion in self.compoundSymbExpression.get("criteria").values()])

    def GetValue(self)->float:
        """Compute criteria value, provide implementation of the composition of criteria

        Returns
        -------
        float
            criteria value
        """
        criteriaBySymbolicName=self.compoundSymbExpression.get("criteria")
        criteriaValues={sy.sympify(symbolName+"(omega)"):criteria.GetValue() for symbolName,criteria in criteriaBySymbolicName.items()}
        compoundValue=self.symbolicCritExpr.evalf(subs=criteriaValues)
        return compoundValue

    def GetSensitivity(self)->np.ndarray:
        """Compute criteria sensitivity, based on the derivative of the composition

        Returns
        -------
        np.ndarray
            criteria sensitivity
        """
        criteriaBySymbolicName=self.compoundSymbExpression.get("criteria")
        criteriaValuesByName={symbolName:criteria.GetValue() for symbolName,criteria in criteriaBySymbolicName.items()}
        criteriaSensitivitiesByName={"Derivative"+symbolName:criteria.GetSensitivity()\
                               for symbolName,criteria in criteriaBySymbolicName.items()
                               }
        criteriaEvalByName={**criteriaValuesByName,**criteriaSensitivitiesByName}
        compoundSensitivities=self.symbolicSensitivFunctor(**criteriaEvalByName)
        return compoundSensitivities

def CheckIntegrity_SymbolicComposedCriteria(GUI=False):
    class DummyCriteria:
        def __init__(self,multFactor):
            __name__="DummyCriteria"
            self.multFactor=multFactor

        def GetValue(self):
            return self.multFactor

        def GetSensitivity(self):
            return self.multFactor*np.ones(10)

    dummy1=DummyCriteria(multFactor=1)
    dummy2=DummyCriteria(multFactor=2)
    dummy3=DummyCriteria(multFactor=3)
    dummy4=DummyCriteria(multFactor=4)

    b=SymbolicComposedCriteria()
    criteriaExpression='2*crit1*crit2-crit3/crit4'
    criteriaByName={"crit1":dummy1,"crit2":dummy2,"crit3":dummy3,"crit4":dummy4}
    b.SetComposedSymbExpression(criteriaExpression=criteriaExpression,criteriaByName=criteriaByName)
    value=b.GetValue()
    expectedValue=2*dummy1.GetValue()*dummy2.GetValue()-dummy3.GetValue()/dummy4.GetValue()
    np.testing.assert_almost_equal(value,expectedValue)

    sensitivity=b.GetSensitivity()
    expectedSensitivity=2*dummy1.GetSensitivity()*dummy2.GetValue()\
                  +2*dummy1.GetValue()*dummy2.GetSensitivity()\
                  - (dummy3.GetSensitivity()*dummy4.GetValue() -dummy3.GetValue()*dummy4.GetSensitivity() )/dummy4.GetValue()**2
    np.testing.assert_almost_equal(sensitivity,expectedSensitivity)
    return "OK"

def CheckIntegrity(GUI=False):
    assert CheckIntegrity_SymbolicComposedCriteria()== "OK"
    return "OK"

if __name__ == "__main__":# pragma: no cover
    print(CheckIntegrity())
