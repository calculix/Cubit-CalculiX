Framework materials
===================

The Framework owns material definitions, workspace material stores and the shared
HDF5 material library. Solver modules do not create Cubit material groups or
register material schemas during startup. Built-in CalculiX definitions currently
cover the existing 17 cards and 41 models.

Data and scope
--------------

A material instance has a workspace-local integer ID and a complete MaterialData
value. MaterialRef combines the workspace and local ID. Each workspace store
validates that scope before accessing an instance. Project identity is available
from CoreFrameworkMaterials for result-source metadata.

Each card contains Active or Inactive state, one selected model, and one payload.
Deactivation preserves that payload. Explicit model replacement clears the old
payload. There is no retained alternative-model data or material revision
history. Expansion reference temperature is optional; an entered zero is a value.

Numeric payloads are canonical SI. Column definitions identify quantities and
optional fields. A missing optional final temperature is represented by a shorter
row; an explicit zero remains present. Table input may therefore contain rows of
different widths when their optional temperature cells differ.

FrameworkMaterialsInterface converts inputs and outputs at a declared boundary:
Canonical, Display or Solver. Display values follow the workspace preferences;
solver values use coherent solver units. Angles are stored in radians and
CalculiX angle parameters are exported in degrees. The Norton creep coefficient
uses the row-dependent dimension stress^(-n) * time^(-(m+1)), with the displayed
stress and time units shown in the coefficient column. Topology base moduli are
also retained in SI and converted when their generated materials are exported.

Commands and GUI
----------------

The shared material editor binds to one workspace and submits its workspace
commands. The application boundary reports failures with CubitMessage. Required
active-card values are validated before activation and export. Inactive drafts
may be saved while incomplete; their shape and numeric values must remain valid.

The legacy property-name accessors are an internal bridge for the existing
CalculiX export and widgets. They compute activation/model-selection values from
typed card state. No USE_CARD properties are stored in a material or HDF5 file.
User-facing commands use card/model names and activate/deactivate actions.
The shared editor retains typed original/draft values, confirms command results
by reading the store before clearing a draft, and rejects stale edits after a
project change. Pending validation errors are tracked per property; editing a
different field does not clear them. Invalid input remains visible while browsing
cards, and must be corrected or discarded before selecting another material.
Table and scalar widgets refresh and reject an edit if their
displayed unit generation changed, avoiding reinterpretation of old numbers.
Library drafts and actions also reject a changed shared-library filename until
the editor is refreshed, preventing a stale selection from editing another file.
Materials in the CalculiX navigation panel and model tree open the same shared
editor through the workspace open command. Window, OK and Close actions use the
workspace close command.

Persistence and library
-----------------------

MaterialProjectPersistence writes complete workspace material definitions and
the next local ID into the caller's project group. Save stages a replacement
group before publishing it; load validates a complete candidate before replacing
the workspace store. A workspace reset clears local materials while preserving
its next ID, so retained result references cannot identify a newly created
material accidentally. Starting a new project replaces the project identity and
resets local ID allocation. The shared external library remains unchanged.

The shared library file is libmaterial.hdf5 beside the Framework component. Its
root is cubit-sim/material-library and its storage schema version is 1. Group and
material indexes, card state, selected models, optional values, canonical values
and row offsets are stored as typed HDF5 datasets. No old native-material format
reader is provided.

Library import creates a new local ID and copies the complete material data.
Further edits to either copy are independent. Export creates a library entry;
replacement is an explicit operation. Library mutations use Hdf5FileTransaction
so the existing file is retained when validation or writing fails. Importing
another Framework library rejects duplicate material identities atomically.

The bundled library was regenerated from the repository's default_materials.jou,
whose example groups declare SI units. It contains the same nine materials in
six groups. Original numeric values, including explicit zero temperatures, are
preserved. These examples retain their original source description.

Implementation
--------------

The source feature is src/Framework/Core/materials and belongs to cubitsim_model.
It depends on the shared unit and HDF5 services, not on any workspace module.
CoreFrameworkMaterials owns the built-in catalog and workspace stores.
CoreFrameworkMaterialLibrary owns shared-file operations.
FrameworkMaterialsInterface is a non-owning workspace facade.
MaterialProjectPersistence encodes typed material values for project and library
storage. Every new material class or struct has its own header/source pair.
