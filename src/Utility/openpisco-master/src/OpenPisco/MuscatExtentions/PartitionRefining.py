# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
#!/usr/bin/python3
# -*- coding: utf-8 -*-
""" 
This module displays a technique called Partition refinement
https://en.wikipedia.org/wiki/Partition_refinement
"""

from typing import Dict,List,Union
import numpy as np


class PartitionRefining():
    """
    Class display a technique representing a partition of a set as a data structure that allows the partition to be refined by splitting its sets into a larger number of smaller sets.
    From here onwards we denote by a family an elementary set, meaning that a so-called group must be the union of several families.
    """
    def __init__(self,cellsByGroupnumber:Dict,cells:Union[np.ndarray,List[int]]):
        """Constructor of the class PartitionRefining

        Parameters
        ----------
        cellsByGroupnumber : Dict
            expressing the association between each group and the cells belonging to them (in FE method either a set of nodes or a set of elements)
        cells : Union[np.ndarray,List[int]]
            array of int cells: set of all cells to be partitionned
        """
        self.cellsByGroupnumber=cellsByGroupnumber
        self.cells=cells
        self.groupsByFamily=dict()
        self.cellsByFamily=dict()

    def GetGroupsByFamily(self)->dict:
        """Get association between groups and family

        Returns
        -------
        dict
            groups by family
        """
        return self.groupsByFamily

    def GetCellsByFamily(self)->dict:
        """Retrieve association between groups and cells

        Returns
        -------
        dict
            cells by family
        """
        return self.cellsByFamily

    def ExecutePartitioning(self):
        """
        Implementation of the algorithm

        Provide a partition of the original set by updating it, cellsByFamily, in a larger number of smaller disjoints sets (minimum required to describe the groups)
        Provide the association between each family and the groups that contain them by updating groupsByFamily
        """
        #initialized to an empty group associated to family 1
        groupsByFamily = {1 : []}
        #initialized to the trivial family 1 that contains every cells
        cellsByFamily={1 : self.cells}
        # At each step of the algorithm, a set X is presented to the algorithm,
        for newgroup, groupcells in self.cellsByGroupnumber.items():
            split_by_family = {}

            for family, familycells in cellsByFamily.items():
                intersect = np.intersect1d(familycells, groupcells, assume_unique=True)
                if len(intersect) == 0:
                    # family and group are disjoint, nothing to do for this family
                    continue
                # family intersects with this group
                if len(intersect) == len(familycells):
                    # family and group are coincident, this is the expected result
                    diff = None
                else:
                    # group is partially intersecting family, need to split this family
                    diff = np.setdiff1d(familycells, groupcells, assume_unique=True)
                split_by_family[family] = (intersect, diff)

            for family, split in split_by_family.items():
                intersect, diff = split
                if diff is not None:
                    cellsByFamily[family] = intersect
                    newfamily = len(cellsByFamily) + 1
                    cellsByFamily[newfamily] = diff
                    groupsByFamily[newfamily] = list(groupsByFamily[family])
                groupsByFamily[family].append(newgroup)

        self.groupsByFamily=groupsByFamily
        self.cellsByFamily=cellsByFamily

   
def CheckIntegrity():
    node_ids = range(1, 9 + 1)
    cellsByGroupnumber = {1:np.unique([1,2,3,4]), 2:np.unique([2,4,5]),3:np.unique([1,6]), 4:np.unique([7,8,9]), 5:np.unique([1,2,3,4,6])}
    PRef=PartitionRefining(cellsByGroupnumber,node_ids)
    PRef.ExecutePartitioning()

    groupsByFamily,cellsByFamily=PRef.GetGroupsByFamily(),PRef.GetCellsByFamily()

    print("cellsByFamily: ",cellsByFamily)
    print("groupsByFamily: ",groupsByFamily)

    from functools import reduce
    for family in groupsByFamily.keys():
        group=tuple(cellsByGroupnumber[elem] for elem in groupsByFamily[family])
        cells=reduce(np.intersect1d,group)
        cells.sort()
        ref=cellsByFamily[family]
        assert cells.all()==ref.all(),"The partition refining has failed"
    return "Ok"

if __name__ == '__main__':
    print(CheckIntegrity())# pragma: no cover
