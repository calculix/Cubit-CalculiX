Mesh, materials, sections and orientations
==========================================

Mesh identities and ownership
-----------------------------

Cubit owns geometry, mesh nodes/elements, blocks, nodesets and sidesets.
``CalculiXMeshInterface`` supplies the project boundary to entity/type queries
and ``CoreBlocks`` maintains CalculiX block/export choices. Query current
connectivity when needed. A global element ID must be resolved to its entity
kind and local ID before calling a kind-specific SDK query.

.. list-table:: Identifier distinctions
   :header-rows: 1
   :widths: 30 70

   * - Identity
     - Interpretation
   * - Block ID
     - A Cubit grouping used for export type/section assignment; not an element ID.
   * - Global element ID
     - A cross-family element identity used by topology and many result maps.
   * - Kind and local ID
     - Pair such as hex/local ID used by SDK connectivity and volume calls.
   * - Nodeset/sideset ID
     - Set identity, not necessarily a node/face/element ID of the same value.
   * - Connectivity position
     - Zero-based local ordering within one element; not a global node number.

``ElementNodeOrdering`` maps CalculiX positions to Cubit/Exodus positions. Export,
integration and result conversion must use consistent permutations, particularly
for quadratic hexes/wedges. Supported families for one feature do not establish
support for every family in every other feature. Topology's current validation
is narrower than the general block element-type list.

Source: :download:`CoreBlocks.hpp <../../src/CalculiX/Core/mesh/CoreBlocks.hpp>`,
:download:`CalculiXMeshInterface.hpp <../../src/CalculiX/Core/mesh/CalculiXMeshInterface.hpp>`
and :download:`ElementNodeOrdering.hpp <../../src/CalculiX/Utility/ElementNodeOrdering.hpp>`.

Materials and material libraries
--------------------------------

``CoreMaterials`` works with Cubit's material interface and the CalculiX property
groups/cards. Material identity, material display name, property-group prefix
and exported solver name are different values. ``get_properties()`` gives a
typed view; export collects the enabled cards and their data. A stored scalar or
table is not sufficient to infer that an inactive card will be emitted.

``CoreMaterialsLibrary`` supports library initialization and management. Library
storage is different from a model's current material assignments. The GUI
material-management widgets are editors and selectors, not independent owners
of the active analysis material model. Preserve the lazy initialization and
module cleanup contracts when adding a new library operation.

Source: :download:`CoreMaterials.hpp <../../src/CalculiX/Core/materials/CoreMaterials.hpp>`
and :download:`CoreMaterialsStructs.hpp <../../src/CalculiX/Core/materials/CoreMaterialsStructs.hpp>`.

Topology material collections
------------------------------

Each topology case has one material collection owned by the material domain.
It contains base-material assignments and per-iteration applied stiffness
factors. It does not require a normal material/tree object for every element.
Group construction controls generated solver materials, preserves base-material
separation and can quantize continuous factors within the configured material
cap. Stored applied factors, requested design values and text-rounded values
must not be confused.

The constitutive scaling model for the supported topology paths is

.. math::

   \mathbf{D}_e=s_e\mathbf{D}_{0,e}.

Density designs request SIMP interpolation; level-set designs request an affine
function of regularized solid fraction. These are not general transformations
of every nonlinear/thermal material card. The supported elasticity assumptions,
grouped-factor sensitivities and historical export behavior are described in
:doc:`topology-export` and :doc:`topology-algorithms`.

Sections
---------

``CoreSections`` has a main ``sections_data`` table linking section ID, section
kind and type-specific record ID. Separate solid, shell, beam and membrane
tables store their parameters. ``get_settings()`` resolves the public ID into a
typed view. Never use a section ID as a row index or assume the solid-section
record ID equals its public section ID.

Normal sections refer to blocks/materials and can carry orientation or other
section-specific data. Shell/membrane thickness and offset, beam profile
parameters and solid-section options are not interchangeable. Export must keep
the appropriate orientation/profile data while selecting the matching keyword.
Model geometry alone does not assign a CalculiX constitutive section.

Topology section collections reference the participating original solid
sections. Their iteration export can create element-level/grouped assignments
without registering a new normal section for every element. Original section
identity and the topology design domain have separate roles; preservation is
resolved through its own set/element rules.

Source: :download:`CoreSections.hpp <../../src/CalculiX/Core/sections/CoreSections.hpp>`
and :download:`CoreOrientations.hpp <../../src/CalculiX/Core/sections/CoreOrientations.hpp>`.

Validation, export and units
----------------------------

Validate entity existence, compatible element type, material assignment and
section kind before preparing an analysis export. Query geometry-derived
volumes from Cubit for topology constraints; element quadrature normalization
has a separate role. A mesh edit or remesh can invalidate IDs/connectivity even
when saved numeric history is retained.

Use consistent model units: Young's modulus is stress, Poisson's ratio is
dimensionless, density is mass/length cubed, thickness and profile dimensions
are lengths. Orientation data changes local axes rather than the global mesh
node IDs. The code writes the selected model values; it does not choose a unit
system on the user's behalf.

When extending a material/section feature, follow its Core default and validation
path, typed settings query, create/modify/delete command, independent editor,
property page, tree routing, persistence and export. Update reference cleanup
when deleting a material, block or orientation referenced elsewhere.
