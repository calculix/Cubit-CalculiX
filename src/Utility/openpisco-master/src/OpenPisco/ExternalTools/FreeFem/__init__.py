# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#


_test= [
"FreeFemInterface",
"FreeFemProblemWriter"
]
def GetScriptsPath():
    """Returns the ScriptsPath of the current module."""
    import os
    return os.path.dirname(os.path.abspath(__file__)) + os.sep
