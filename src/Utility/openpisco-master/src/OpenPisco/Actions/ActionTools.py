# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from typing import Dict

from Muscat.MeshContainers.Mesh import Mesh

def GetSupportFromLsOrSupport(ops: Dict) -> Mesh :
    """Retrieve mesh support from either a levelset or a support

    Parameters
    ----------
    ops : Dict
        parameters

    Returns
    -------
    Mesh
        Support
    """
    assert ('ls' in ops and 'support' not in ops) or ('ls' not in ops and 'support' in ops), "Need to provide either ls or support"
    support = ops['ls'].support if 'ls' in ops else ops['support']
    return support

def CheckIntegrity(GUI : bool= False):
    return "ok"

if __name__ == "__main__":  # pragma: no cover
    print(CheckIntegrity(GUI=True))

