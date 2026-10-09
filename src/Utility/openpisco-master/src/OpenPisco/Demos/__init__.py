# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

_test = []


def GetDemoPath():
    """Returns the testDataPath of the current module."""
    import os
    return os.path.dirname(os.path.abspath(__file__)) + os.sep
