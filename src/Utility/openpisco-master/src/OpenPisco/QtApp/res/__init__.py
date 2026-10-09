# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

def ResourcePath():
    import os
    import OpenPisco.QtApp
    return os.path.dirname(os.path.abspath(OpenPisco.QtApp.__file__)) + os.sep + "res" + os.sep
