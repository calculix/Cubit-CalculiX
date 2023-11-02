# Cubit-CalculiX
Hey ho that´s the CalculiX Component for coreform cubit!

Full Model definitions for CalculiX can be done with Cubit as Preprocessor.
After the run with CalculiX the results will automatically converted for Paraview.

Here you can find the source code and a build for linux.
 
This component was build with Cubit 2023.8! 

Tested with Cubit 2023.8 and 2023.10 on Ubuntu 22.04

# Key Features
Definition of:
- Elements Sets
- Node Sets
- Side Sets
- Materials (Conductivity, Density, Elastic, Expansion, Plastic, Specific Heat)
- Sections (Solid, Shell, Membrane, Beam)
- Constraints (Tie, Rigid Body)
- Surface Interactions
- Contact Pairs
- Amplitudes
- Loads (CLOAD, DLOAD, DFLUX, GRAVITY, CENTRIF)
- Boundary Conditions (Displacements, Temperatures)
- Initial Conditions (Displacements, Temperatures)
- History Outputs (NODE PRINT, EL PRINT, CONTACT PRINT)
- Field Outputs (NODE FILE, EL FILE, CONTACT FILE)
- STEPS (Static, Frequency, Buckle, Heat Transfer, Coupled Temperature Displacement, Uncoupled Temperature Displacement, No Analysis)
- Creation and Running of Jobs
- Conversion of Results to Paraview (.frd and .dat: nodal, element and integration points results)
- Projection of Displacements from .frd to mesh
- Opening of Results with CGX or Paraview
- Adding Customlines before/after Keywords

- Export of Calculix Model

# Installation

Download the build directory and point within Cubit to the Direcetory.

- Tools->Plugins->Add

Restart Cubit.

![Add Component](/doc/Setup_00.png?raw=true "Add Component")

The Cubit Window should now show "Coreform Cubit 2022.4 -- with CalculiX" in the Title.

- Tools->Options->CalculiX

Now you have to fill in the Paths to CCX, CGX, Paraview, ccx2paraview, the icons from the build and give a number for the wanted threads for the solver.

Restart Cubit.

![Setup Options](/doc/Setup_01.png?raw=true "Setup Options")

Enable Model Tree

- View->CalculiX Model Tree

![Show Model Tree](/doc/Setup_02.png?raw=true "Show Model Tree")

Try to open and run the journal file "first_run.jou"

If everything works. It should compute a beam and automatically open the results with cgx and paraview.

![After First Run](/doc/Setup_03.png?raw=true "After First Run")

# Postprocessing

After the run with CalculiX the results should be loaded and converted automatically. If a Job exited with errors you can try to convert it over the Button in the Job Monitor.
![convert](/doc/postprocessing_01.png?raw=true "convert")

The mises stresses and strains will be precalculated for the S and E keys.
If the preprocessing was done in cubit and the results can be linked. Then the data from the .frd and .dat can be viewed with paraview. If a linking is not successful, then only .frd will be converted to vtu.
In paraview you can see the linked and computed results with the multiblock inspector.

![multiblock inspector](/doc/postprocessing_02.png?raw=true "multiblock inspector")

If integration point data was requested in the .dat, the converter computes the location of the integration points and links the results. The integration point number and the element id can be queried with paraview.

![integration points](/doc/postprocessing_03.png?raw=true "integration points")

When there are displacements in the frd file for requested blocks in the .dat. The displacements of the integration points will also be computed.

If integration point data was requested in the .dat, the converter computes the location of the integration points and links the results. The integration point number and the element id can be queried with paraview.

![integration points displacements](/doc/postprocessing_04.png?raw=true "integration points displacements")

current limitations of the converter:
- CELS key ignored
- contact print card ignored
- totals ignored
- buckling .dat ignored
- only links for complete nodal and element data

# Projection of Displacements

with the command "ccx result project job <job_id> {step <step>|totalincrement <totalincrement>} [scale <scale>]"

The displacements from an .frd can be scaled and projected to the nodes in cubit. It has to be .frd where the mesh is ident with the one in cubit.
Otherwise linking fails and no projection will be done. If a step is choosen, it will project the last available increment with displacement data.
When choosing the totalincrement the related displacements will be used.

![projection of displacements](/doc/projection_01.png?raw=true "projection of displacements")

# known issues
If you want to build the component yourself, i would recommend using ubuntu 22.04 . If some symbols are missing try another OS.
It can happen that you've got a Cubit which was build on a different OS. For example building the component for Cubit 2022.4 only worked with centos 7.

# todo list
- documentation
