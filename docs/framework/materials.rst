Materials
=========

The Framework owns material data and the shared HDF5 material library. Each
solver workspace has its own material IDs and complete local definitions saved
with the project. Library entries are copied into the chosen workspace; editing
the copy does not change the library entry.

Open the shared editor with the Materials toolbar button, the Materials
navigation entry, or the workspace command::

  ccx open materialmanagement
  ccx open materialmanagement material 1
  ccx close materialmanagement

The editor retains the existing material and library layout. Each material card
has one selected model and Active or Inactive state. Deactivating a card keeps
its values and excludes it from solver export. Changing its model replaces the
previous model's values. Inactive cards may be saved before all required values
are entered; activating or exporting a card validates its required values.
Creating an empty material with ``ccx create material`` installs its supported
cards in the Inactive state. Select the model, enter values and activate the
required cards. Use ``add card`` when recreating a card that was removed, rather
than adding a card already present.

Units and values
----------------

Material values are stored in SI and displayed using workspace unit preferences.
The default N-mm-s workspace displays stress in N/mm^2 and density in kg/m^3.
Solver export converts values to the selected coherent solver unit system.
Changing display units does not change the physical material values.

Unit-bearing table headings use compact notation such as ``N/mm²`` and ``°C``.
Their tooltips retain the original input unit expression. Cells display readable
numbers such as ``0.3`` and ``273.15`` instead of exposing binary representation
tails. An unedited cell retains its original stored precision when other cells
are edited or Apply is used. An edited cell is parsed from the new input.
See :doc:`units` for display preferences and command syntax.

Optional temperature cells may be left blank. A zero is a real temperature value
and is preserved. Expansion reference temperature is also optional. If units are
changed while the editor is open, an edit based on the previous units is rejected
and the fields refresh before further input.

Material commands
-----------------

Commands use the active solver workspace prefix. For CalculiX, these commands
create a material, assign an isotropic elastic model and activate its card::

  ccx create material "Steel"
  ccx modify material 1 card elastic model isotropic
  ccx modify material 1 card elastic row 1 values 210000 0.3
  ccx modify material 1 activate card elastic

Without an explicit unit option, numeric input uses the workspace display units
for each field. ``unit "SI"`` declares canonical SI input for an entire row::

  ccx modify material 1 card elastic row 1 values 210000000000 0.3 unit "SI"
  ccx modify material 1 deactivate card elastic
  ccx modify material 1 activate card elastic
  ccx modify material 1 name "Structural Steel"

Replacing a model clears its old values. Clearing values keeps the selected
model. Removing a card removes its state and payload::

  ccx modify material 1 deactivate card elastic
  ccx modify material 1 card elastic model engineering_constants
  ccx modify material 1 card elastic clear values
  ccx modify material 1 remove card elastic

Shared library
--------------

The shared file is ``libmaterial.hdf5`` in the Framework component directory by
default. Configure the Framework library file option to use another shared file::

  ccx materiallibrary file "C:/materials/libmaterial.hdf5"

Groups organize entries. The library retains the solver schema associated with
each entry, and material IDs remain local to the target workspace.

Import copies a library entry into a workspace and assigns a new local ID::

  sim import materiallibrary name "S235" groupname "Metals/Steel" workspace calculix material_name "Steel"

Export copies a local material into the library::

  ccx export materiallibrary material_name "Structural Steel" name "Steel" groupname "Metals/Steel"

Library material edits use the same card actions after a named library target::

  ccx modify materiallibrary material name "Steel" groupname "Metals/Steel" deactivate card elastic

The bundled example library contains the existing nine materials in six groups,
preserving their source descriptions and explicit SI values. The accompanying
``journal_files/default_materials.jou`` populates an empty library only. Previous
native Cubit material data and the old library format are not converted.

Section material references
---------------------------

CalculiX section creation and modification validate material IDs against the
CalculiX workspace store before changing assignments. Solid, shell, beam and
membrane export resolve material names through that same store, including
solver-specific name prefixes. Native Cubit material IDs never satisfy these
references. Missing references produce Cubit errors.

Example material units
----------------------

The migrated journals and Python example select N-mm-s for CalculiX and write
material rows with ``unit "SI"``. This makes their physical meaning independent
of saved display preferences. Stress is entered in Pa, density in kg/m^3,
specific heat in J/(kg K), conductivity in W/(m K), and temperature in kelvin.
The solver exporter converts these values to the coherent N-mm-s basis.
For example, 7800 kg/m^3 becomes 7.8e-9 tonne/mm^3, 500 J/(kg K) becomes
5e8 N mm/(tonne K), and 210 GPa becomes 210000 N/mm^2.

Old illustrative density values 0.0078, 7.9e-6 and 8 have been corrected to
7800, 7900 and 8000 kg/m^3 respectively. Specific heat values 0.5 and 5e8
have been corrected to 500 J/(kg K). Existing SI temperature-dependent tables
retain their physical properties; Celsius table temperatures are converted to
kelvin. These examples are demonstrations, not certified material datasets.

Material creation uses ``ccx create material``. Table rows use readable card
commands, and imported materials use ``sim import materiallibrary`` with an
explicit destination workspace. Unpopulated legacy card-selection switches
do not create empty active cards.



The Materials icon is the penultimate action on the CalculiX workspace toolbar,
followed by Units. Postprocessor and Report do not show a Materials action.
The shared management window and workspace-local material IDs are unchanged.
