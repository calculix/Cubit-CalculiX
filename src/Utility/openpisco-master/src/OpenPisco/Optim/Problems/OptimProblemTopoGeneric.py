# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import annotations

from typing import Dict, Optional

from Muscat.Helpers.Decorators import froze_it
import Muscat.Helpers.ParserHelper  as PH
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.MeshTools.MeshInspectionTools import ComputeMeshMinMaxLengthScale

from OpenPisco.Optim.Problems.ProblemFactory import RegisterClass as RegisterProblemClass
from OpenPisco.Optim.Problems.OptimTopoProblemBase import OptimTopoProblemBase
from OpenPisco.Optim.Problems.OptimProblemConcrete import OptimProblemConcrete
from OpenPisco.Optim.Criteria.CriteriaFactory import Create as CreateCriteria
from OpenPisco.Optim.Criteria.ComposedCriteria import SymbolicComposedCriteria
from OpenPisco.Optim.Criteria.SemiInfiniteCriteria import SemiInfiniteCriteriaBase
from OpenPisco.Optim.Criteria.Criteria import CriteriaBase

def AddInequalityConstraintToOptimProblem(POptim:OptimProblemConcrete,criterion:CriteriaBase):
    """Add inequality constraint to optimization problem. 
    For semi-infinite criteria, we use a slightly different approach by relying on their discrete implementation for adding individual constraints.

    Parameters
    ----------
    POptim : OptimProblemConcrete
        optimization problem
    criterion : CriteriaBase
        criterion to add as an inequality constraint
    """
    if isinstance(criterion,SemiInfiniteCriteriaBase):
        newCriteria = criterion.GetDiscreteImplementation()
        for newCriterion in newCriteria:
            POptim.AddInequalityConstraint(newCriterion)
    else :
        POptim.AddInequalityConstraint(criterion)

@froze_it
class OptimProblemTopoGeneric(OptimTopoProblemBase):
    def __init__(self, other:Optional[OptimProblemTopoGeneric]=None, dataDeepCopy:bool=True):
        """Class constructor, this class is immutable 

        Parameters
        ----------
        other : Optional[OptimProblemTopoGeneric], optional
            another instance of this very problem to copy attributes from, by default None.
        dataDeepCopy : bool, optional
            whether to use deepcopy, by default True
        """
        super(OptimProblemTopoGeneric, self).__init__(other=other)

#####
def CreateOptimProblemTopoGeneric(ops:Dict):
    """Create optimization problem from its components

    It allows to define the complete optimization problems; here are the main ingredients:
    - Objective (mandatory): criteria to be minimized
    - Constraint (optional): there are 2 options:
        - Inequality constraint, either with a upperbound or lowerbound
        - Equality constraint with a target value
    - Criterion (optional): it is a special case where one would like to evaluate a behaviour of a criterion 
    through the optimization process without trying to optimize it at all
    - ExportExtraFields (optional): allow to export either a scalar quantity or a field computed by a physical solver.

    To illustrate that in the XML format, we provide a small snippet where we:
    - minimize the volume 
    - with a compliance inequality constraint
    - export the potential energy at nodes from the physical problem

    <PhysicalProblems>
        <Unstructured type="static_elastic" id="1" dim="2">
            <Material  E="210.e9" Nu="0.3" />
            <Dirichlet eTag="X0" dofs="0 1" value="0.0"/>
            <Force  nTag="nTag4" dir="0 1" value="-0.003"/>
         </Unstructured>
    </PhysicalProblems>

    <OptimProblems>
        <OptimProblem type="TopoGeneric" id="1">
          <Objective  type="Volume"  name="Volume"/>
          <Constraint type="Compliance"  name="Compliance" UpperBound="3*1.90489800e-04" useProblem="1" />
          <ExportExtraFields useProblem="1" fields="potential_energy" locations="Nodes"/>
       </OptimProblem>
    </OptimProblems>

    Parameters
    ----------
    ops : Dict
        optimization problem components

    Returns
    -------
    OptimProblemTopoGeneric
        OptimProblemTopoGeneric instance endowed with the optimization problem components

    Raises
    ------
    ValueError
        Constraint must be provided with either a lowerbound, upperboun or target value
    ValueError
        Only Objective, Constraint and Criterion are supported
    ValueError
        Feeature not supported
    """
    res = OptimProblemTopoGeneric()
    for typeName,data in ops["children"]:
        if "type" in data:
            criteriaType = data['type']
            del data['type']
            crit = CreateCriteria(criteriaType,data)
            if typeName == "Objective":
                res.SetObjectiveCriterion(crit)
            elif typeName == "Constraint":
                if "UpperBound" in data or "upperBound" in data:
                   AddInequalityConstraintToOptimProblem(res,crit)
                   res.SetInequalityConstraintStatus(res.GetNumberOfInequalityConstraints()-1,True)
                elif "LowerBound" in data:
                   data["UpperBound"] = -data.pop("LowerBound")
                   newCrit=SymbolicComposedCriteria()
                   newCrit.SetComposedSymbExpression(criteriaExpression='-crit1',criteriaByName={"crit1":crit})
                   AddInequalityConstraintToOptimProblem(res,newCrit)
                   res.SetInequalityConstraintStatus(res.GetNumberOfInequalityConstraints()-1,True)
                elif "TargetValue" in data or "targetValue" in data:
                   res.AddEqualityConstraint(crit)
                   res.SetEqualityConstraintStatus(res.GetNumberOfEqualityConstraints()-1,True)
                else:
                    raise ValueError("Error: need an UpperBound, LowerBound or a TargetValue")
            elif typeName == "Criterion":
                res.AddInequalityConstraint(crit)
                res.SetInequalityConstraintStatus(res.GetNumberOfInequalityConstraints()-1,False)
            else:
                raise ValueError("The type " + str(typeName)+ " is not available ")
        elif typeName == "ExportExtraFields":
            try:
                problem = data["problem"]
            except KeyError:
                raise KeyError("Can not add field to export without problem")
            
            if "fields" in data and "locations" in data:
                fieldNames,locations=PH.ReadStrings(data["fields"]),PH.ReadStrings(data["locations"])
                assert len(fieldNames)==len(locations),"Number of fields to export should match numer of locations"
                res.AddProblemFields(problem=problem,
                                     fieldNames=fieldNames,
                                     locations=locations)
            if "scalars" in data:
                res.AddProblemScalars(problem=problem,scalarNames=PH.ReadStrings(data["scalars"]))
        else:
            raise ValueError("typeName" + str(typeName) + "not handled" )


    del ops["children"]

    res.writer = ops.get("writer",None)
    if res.writer is not None:
        del ops["writer"]

    res.point = ops.get("point",None)
    if res.point is not None:
        del ops["point"]


    if "e2" not in ops:
        if res.point.support.props.get("IsConstantRectilinear",False):
            ops["e2"] = 3 * min(res.point.support.props.get("spacing")) **2
        else:
            ops["e2"]  = 3 * ComputeMeshMinMaxLengthScale(res.point.support)[0] **2
        print("Setting lengthscale regularization parameter automatically : " + str( ops["e2"]))

    for key in list(ops.keys()):
        if "maxChange" in key:
            print(key)
            res.maxChange[key[9:]] = PH.ReadFloat(ops[key])
            del ops[key]

    if "OnTags" in ops:
        res.OnFilter = ElementFilter()
        res.OnFilter.eTags = ops.get("OnTags","").split()
        del ops["OnTags"]

    if "OffTags" in ops:
        res.OffFilter = ElementFilter()
        res.OffFilter.eTags = ops.get("OffTags","").split()
        del ops["OffTags"]

    PH.ReadProperties(ops,ops,res)

    return res
#####
RegisterProblemClass("TopoGeneric", OptimProblemTopoGeneric,CreateOptimProblemTopoGeneric)

def CheckIntegrity():
    return 'ok'

if __name__ == '__main__':# pragma: no cover
    print(CheckIntegrity())
