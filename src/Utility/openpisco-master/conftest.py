# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE.txt', which is part of this source code package.
#
#

import pytest

@pytest.hookimpl(tryfirst=True)
def pytest_pyfunc_call(pyfuncitem):
    testfunction = pyfuncitem.obj
    res = testfunction()
    if any([res.lower().startswith(x) for x in ["ok", "skip"]]):
        return True
    raise Exception(res)

def pytest_pycollect_makeitem(collector, name, obj):
    if name == "CheckIntegrity" and hasattr(obj, "__call__"):
        # Collect CheckIntegrity functions
        #if hasattr(pytest.Function, "from_parent"):
        return   pytest.Function.from_parent(collector, name=name)
    else:
        # Fallback to default handler
        return None
