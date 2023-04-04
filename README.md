# Cubit-CalculiX
Hey ho that´s the CalculiX plugin for coreform cubit!

Here you can find the source code and a build for linux.
If you want to build the plugin yourself, i would recommend using Centos 7.
As far as i know, that's the distro on which cubit is build.
This plugin was tested with Cubit 2022.4 on Ubuntu 22.04

# Upcoming soon
I am currently working on a deeper integration of calculix to cubit. So that a complete model definition can be done with cubit.
When the alpha is ready i will release it here.

# Installation

copy the **libcalculix_plugin.so** into the plugins directory, that you can find in your Cubit installation directory.

e.g.: /opt/Coreform-Cubit-2022.4/bin/plugins/

Features:
- adds an export option for the use with CalculiX
- supported element types: see listing below 
- supports export of nodesets and sidesets
- supports export of materials and section properties

How to use:
- command 'export ccx "filename"' for export of the mesh
- command 'ccx print core' to print out data from the core, mainly useful for development
- command 'ccx log core' to print out data from the core to the logfile, mainly useful for development
- command 'ccx version' to print out the plugin version
- command 'ccx block <block id>... element_type <ccx element type>' set the ccx element type to a block

# Standard Element conversion:
```
Cubit       -> CalculiX
------------------------
SPHERE      ->  Element will not be exported. Only Node in ALLNODES remains
BAR         ->  B21
BAR2        ->  B21
BAR3        ->  not supported
BEAM        ->  B31
BEAM2       ->  B31
BEAM3       ->  B32
TRUSS       ->  T3D2
TRUSS2      ->  T3D2
TRUSS3      ->  T3D2
SPRING      ->  SPRINGA
TRI         ->  CPS3
TRI3        ->  CPS3
TRI6        ->  CPS6
TRI7        ->  not supported
TRISHELL    ->  S3
TRISHELL3   ->  S3
TRISHELL6   ->  S6
TRISHELL7   ->  not supported
SHELL       ->  S4R
SHELL4      ->  S4R
SHELL8      ->  S8R
SHELL9      ->  not supported
QUAD        ->  CPE4R
QUAD4       ->  CPE4
QUAD5       ->  not supported
QUAD8       ->  CPE8R
QUAD9       ->  not supported
Tetra       ->  C3D4
Tetra4      ->  C3D4
Tetra8      ->  not supported
Tetra10     ->  C3D10
Tetra14     ->  not supported
Tetra15     ->  not supported
HEX         ->  C3D8R
HEX8        ->  C3D8
HEX9        ->  not supported
HEX20       ->  C3D20
HEX27       ->  not supported
```

# material and section properties export:
Use the Cubit-Abaqus Card to define the material.

Supported Material Properties:
 - youngs modulus
 - poisson's ratio
 - yield stress vs. strain vs. temperature

The thickness for plain strain and stress elements is defined with block attribute 1

The thickness for shells is defined with block attribute 1, the offset with block attribute 2

Beam section not supported.
