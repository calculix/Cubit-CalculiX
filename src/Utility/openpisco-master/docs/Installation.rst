.. This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.

Installation
============

Dependencies
------------------------


.. csv-table:: OpenPisco dependencies
   :header: "Dependency", "Use"
   :widths: 20, 40

   "python_ >=3.10", "mandatory"
   "muscat_", "mandatory"
   "numba_", "mandatory"
   "osqp_", "mandatory for using the null space optimization algorithm"
   "pyqtgraph_", "mandatory for using the GUI application OpenPisco"
   "pyside2_", "mandatory for using the GUI application OpenPisco"
   "qtconsole_", "mandatory for using the GUI application OpenPisco"
   "Mmg_", "mandatory for remeshing and running body-fitted topology optimization"
   "Advection_", "mandatory for using the level set advection algorithm"
   "Mshdist_", "mandatory for using the level set redistancing algorithm"
   "Code_Aster_", "optional finite element external software"
   "Freefem_", "optional finite element external software"
   "ZSet_", "optional finite element external software"
   "OpenFOAM_", "optional cell-centered finite volume external software"
   "libmed_", "mandatory for using the Code_Aster interface"

.. _python: https://www.python.org/
.. _muscat: https://gitlab.com/drti/muscat
.. _numba: https://numba.pydata.org/
.. _osqp:  https://osqp.org/
.. _pyqtgraph : https://www.pyqtgraph.org/
.. _pyside2 : https://pypi.org/project/PySide2/
.. _qtconsole : https://pypi.org/project/qtconsole/
.. _Code_Aster : https://www.code-aster.org
.. _Freefem : https://freefem.org/
.. _Zset : http://www.zset-software.com/
.. _OpenFOAM : https://openfoam.org/
.. _Mmg : https://www.mmgtools.org/
.. _Mshdist: https://github.com/ISCDtoolbox/Mshdist
.. _Advection : https://github.com/ISCDtoolbox/Advection
.. _libmed : https://github.com/conda-forge/libmed-feedstock


Install the last stable release with conda/mamba
-------------------------------------------------

Users can install the minimal version with the conda package with

.. code-block:: bash

    mamba create -n OpenPiscoBaseEnv --override-channels -c conda-forge openpisco-core=1.4.1

Users can install openpisco together with dependencies (except for finite element external solvers) with

.. code-block:: bash

    mamba create -n OpenPiscoEnv --override-channels -c conda-forge openpisco=1.4.1


So far, OpenPisco has been successfully installed and tested with the following operating system types:

* Linux: Ubuntu
* MacOs
* Windows: 10 and wsl (20.04)


.. note::
   Some optional dependencies (iscd-mshdist,iscd-advection) are not packaged for Windows as of now.
   For Windows users, a convenient alternative could be WSL, for instance.


Install external finite element solvers
-------------------------------------------
External finite element solvers (Code_Aster,FreeFem,Zset) must be installed and available from the command line in order to interface them with OpenPisco.

Install Code_Aster
"""""""""""""""""""""
The package libmed is needed for using the OpenPisco interface to Code_Aster and can be installed with

.. code-block:: bash

    conda install -c conda-forge libmed

To install Code_Aster it is recommended to visit the `Code_Aster installation guide <https://code-aster.org/V2/spip.php?rubrique21>`_.
Alternatively, a Code_Aster conda package is available within the conda-forge and can be installed with


.. code-block:: bash

    conda install -c conda-forge code-aster

To avoid conflicts between Code_Aster and your current conda environment, it is recommanded to have a dedicated conda environment for Code_Aster alone.
Thus, in order to be able to use this solver, the only remaining step is to define a suitable environment variable.
Let YourStandaloneAsterEnv be the name of your dedicated conda environment, then

.. code-block:: bash

    export ASTER_EXECUTABLE="conda run -n YourStandaloneAsterEnv as_run"

For issues regarding the installation of Code_Aster it is strongly recommended to contact the Code_Aster maintainers.


Install FreeFem
"""""""""""""""""""""
To install FreeFem it is recommended to visit the `FreeFem installation guide <https://doc.freefem.org/introduction/installation.html>`_ .
Alternatively, a FreeFem conda package is provided within the `OpenPisco anaconda channel <https://anaconda.org/openpisco>`_ and can be installed with

.. code-block:: bash

    conda install -c openpisco freefem

To avoid conflicts between FreeFem and your current conda environment, it is recommanded to have a dedicated conda environment for FreeFem alone.
Thus, in order to be able to use this solver, the only remaining step is to define a suitable environment variable.
Let YourStandaloneFreefemEnv be the name of your dedicated conda environment, then

.. code-block:: bash

    export FREEFEM_EXECUTABLE="conda run -n YourStandaloneFreefemEnv FreeFem++"

For issues regarding the installation of FreeFem it is strongly recommended to contact the FreeFem maintainers.


Install ZSet
"""""""""""""""""""""
To install ZSet it is recommended to visit the `ZSet installation guide <http://www.zset-software.com/downloads/versions/>`_.
For issues regarding the installation of ZSet it is strongly recommended to contact the Zset maintainers.


Install external finite volume solver
-------------------------------------------
External cell-centered finite volume solver (OpenFOAM) must be installed and available from the command line in order to interface them with OpenPisco.


Install OpenFOAM
"""""""""""""""""""""
To install OpenFOAM v2412, it is recommended to visit the `Compiling OpenFOAM from Source Code <https://develop.openfoam.com/Development/openfoam/blob/develop/doc/Build.md>`_.
Alternatively, an OpenFOAM precompiled package is available for several OS and it is recommended to visit the `Installing OpenFOAM from apt package <https://develop.openfoam.com/Development/openfoam/-/wikis/precompiled/>`_.

Another option, an OpenFOAM conda package is available within the conda-forge and can be installed with


.. code-block:: bash

    conda install -c conda-forge openfoam 

To avoid conflicts between OpenFOAM and your current conda environment, it is recommanded to have a dedicated conda environment for OpenFOAM alone.
Thus, in order to be able to use this solver, the only remaining step is to define a suitable environment variable. 
Let YourStandaloneOpenfoamEnv be the name of your dedicated conda environment, then

.. code-block:: bash

    export OPENFOAM_EXECUTABLE="conda run -n YourStandaloneOpenfoamEnv"

For issues regarding the installation of OpenFOAM it is strongly recommended to contact the OpenFOAM maintainers.


Testing
=======

To test your installation, activate the OpenPiscoEnv environment and run

.. code-block:: bash

   python -m Muscat -e OpenPisco


Note that some tests can be disabled using an environment variable, for example when a test relies on an external dependency that may not be available **on your system/environment**. The feature relies on the definition of non-empty environment variables of the form:

.. code-block:: bash

   "appsname_NO_FAIL"

where appsname can take one of the following values

* ZSET
* ASTER
* MED
* FREEFEM
* FOAM

Alternatively, in order to test development branch, one can use the `Pipeline script <https://gitlab.com/openpisco/openpisco/-/blob/master/extras/pipeline/Pipeline.py>`_.
For that, the package furo is required

.. code-block:: bash

    conda install conda-forge::furo

It might also be required to overrides all the other localisation settings

.. code-block:: bash
    
    export LC_ALL=C

Such a script generates a report in html containing results of several demo scripts (final shape, criteria/memory evolution though the optimization process etc...). 
While its main purpose is to assess whether there is a regression with respect to a reference branch, it can also be used to display easily results on several usecases.