# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

from setuptools import setup

setup(
    include_package_data=True,
    package_data={'': ['TestData/*','TestData/*/*','TestData/*/*/*','QtApp/res/*.svg',"Demos/*/*.xml","ExternalTools/Aster/*.comm", "ExternalTools/Aster/*.export", "ExternalTools/FreeFem/Macros/*.edp" ]}
    )
