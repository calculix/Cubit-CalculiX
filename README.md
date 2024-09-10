# Cubit-CalculiX 2024.9
Hey ho that´s the CalculiX Component for coreform cubit!

Full Model definitions for CalculiX can be done with Cubit as Preprocessor.
After the run with CalculiX the results will be automatically converted for Paraview.

Here you can find the source code.

The builds can be downloaded at our [website](https://www.maschinenbauer.at/open-source/calculix-component-for-coreform-cubit).

Or just use the links.

[Windows Build](https://www.maschinenbauer.at/upload/build-windows-2024-9.zip)
[Linux Build](https://www.maschinenbauer.at/upload/build-linux-2024-9.zip)

This component was build with Cubit 2024.8! 

Tested with Cubit 2024.8 on Windows 10 and Ubuntu 22.04

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
- Orientations
- Loads (CLOAD, DLOAD, DFLUX, GRAVITY, CENTRIF)
- Boundary Conditions (Displacements, Temperatures)
- Initial Conditions (Displacements, Temperatures)
- History Outputs (NODE PRINT, EL PRINT, CONTACT PRINT)
- Field Outputs (NODE FILE, EL FILE, CONTACT FILE)
- STEPS (Static, Frequency, Buckle, Heat Transfer, Coupled Temperature Displacement, Uncoupled Temperature Displacement, No Analysis)
- Adding Customlines before/after Keywords
- Export of Calculix Model
- Creation and Running of Jobs
- Conversion of Results to Paraview (.frd and .dat: nodal, element and integration points results, auto calculating von Mises and Principal Stresses)
- Projection of Displacements from .frd to mesh
- Opening of Results with CGX or Paraview
- Results can be viewed, plotted and exported to .csv
- Displaying of loads and boundary conditions
- Python Interface to query the result files. This can be used to run convergence studies or do mesh refinement based on the results. 

# How to
To get an overview of all new commands type

ccx ?

in cubit mode (#!cubit)

For the functions of the python interface type

ccx.help()

in python mode (#!python)

# Examples
[<img
  src="examples/first_run.gif"
  width="400"
  title="First run example">
](examples/)
[<img
  src="examples/bolted_connection/example.gif"
  width="400"
  title="Simple bolted connection">
](examples/bolted_connection/)
[<img
  src="examples/snap_fit/example.gif"
  width="400"
  title="Simple snap fit">
](examples/snap_fit/)
[<img
  src="examples/contact_press_fitting/example.png"
  width="400"
  title="Contact press fitting">
](examples/contact_press_fitting/)
[<img
  src="examples/mesh_refinement/example.png"
  width="400"
  title="Mesh refinement">
](examples/mesh_refinement/)
[<img
  src="examples/model_change/example.gif"
  width="400"
  title="Custom Key Words - Model Change">
](examples/model_change/)
[<img
  src="examples/nonlinear_buckling_cylindrical_shell/example.gif"
  width="400"
  title="Buckling cylindrical shell">
](examples/nonlinear_buckling_cylindrical_shell/)
[<img
  src="examples/simple_convergence_study/example.png"
  width="400"
  title="Simple convergence study">
](examples/simple_convergence_study/)
[<img
  src="examples/Welding/shrinkage_model/example.gif"
  width="400"
  title="Shrinkage model">
](examples/Welding/shringage_model/)

# Installation

Download the build directory and point within Cubit to the Directory.

- Tools->Plugins->Add

Restart Cubit.

![Add Component](/readme_docs/Setup_00.png?raw=true "Add Component")

The Cubit Window should now show "Coreform Cubit 202x.x -- with CalculiX" in the Title.

Now just enable the Model Tree

- View->CalculiX Model Tree

![Show Model Tree](/readme_docs/Setup_01.png?raw=true "Show Model Tree")

Try to open and run the journal file "first_run.jou"

If everything works. It should compute a beam and automatically open the results with cgx and paraview.

![After First Run](/readme_docs/Setup_02.png?raw=true "After First Run")

# Postprocessing

After the run with CalculiX the results should be loaded and converted automatically. If a Job exited with errors you can try to convert it over the Button in the Job Monitor.

The mises stresses and strains will be precalculated for the S and E keys.
If the preprocessing was done in cubit and the results can be linked. Then the data from the .frd and .dat can be viewed with paraview. When the linking fails, the .frd will be converted to vtu.
In paraview you can see the linked and computed results with the multiblock inspector.

![multiblock inspector](/readme_docs/postprocessing_01.png?raw=true "multiblock inspector")

If integration point data was requested in the .dat, the converter computes the location of the integration points and links the results. The integration point number and the element id can be queried with paraview. The element id can be found in the cell data. The ip number in the point data.

![integration points](/readme_docs/postprocessing_02.png?raw=true "integration points")

![elements id](/readme_docs/postprocessing_03.png?raw=true "element id")

![ip number](/readme_docs/postprocessing_04.png?raw=true "ip number")

When there are displacements in the frd file for requested blocks in the .dat. The displacements of the integration points will also be computed.


![integration points displacements](/readme_docs/postprocessing_05.png?raw=true "integration points displacements")

current limitations of the converter:
- totals ignored
- buckling in .dat ignored
- contact print card ignored
- only links for complete nodal and element data

# Projection of Displacements

with the command "ccx result project job 'job_id' {step 'step' | totalincrement 'totalincrement'} [scale 'scale']"

The displacements from an .frd can be scaled and projected to the nodes in cubit. It has to be .frd where the mesh is ident with the one in cubit.
Otherwise linking fails and no projection will be done. If a step is choosen, it will project the last available increment with displacement data.
When choosing the totalincrement the related displacements will be used.

![projection of displacements](/readme_docs/projection_01.png?raw=true "projection of displacements")

# known issues
- Can not open results for Paraview 5.12.0 out of Cubit. Please use Paraview 5.11.* or a new Version with a fix. See https://gitlab.kitware.com/paraview/paraview/-/issues/22614

- History Bug on Windows: Currently commands that will be send using the gui won't be recorded into the history tab. This has been fixed for Linux. In the current windows release (2024.8) are some symbols missing. So we have to wait onto the next release to fix this for windows too.

# todo list
- documentation
