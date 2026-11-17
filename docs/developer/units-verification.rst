Units stage verification
========================

Scope and close-out status
--------------------------

This record closes the documented Windows units stage for the checks listed
below. It separates automated evidence, user-confirmed GUI observations and
source contracts. It does not certify every load, solver formulation, output
component or workspace workflow.

**Linux automated and manual verification: PENDING.**
Windows results must not be presented as Linux results.

The retained automated report is local run
``20261116T132856Z_3f89f3cf`` from 16 November 2026. Its final status is
``AUTOMATED_CHECKS_PASS_MANUAL_PENDING``. Manual confirmations supplied on
17 November subsequently cover checks 1--5 and the compact labels and headings.
Those confirmations supplement the automated report; its original status and
contents have not been rewritten.

The report records Coreform Cubit 2026.11 as the configured SDK installation
and Qt 6.8.3 as the selected Qt installation. The loaded plugin revision and
solver executable revision were not recorded in that report. A checkout hash
or another run's solver version must not be substituted for them.

The automated run predates the Hz-default assertion and compact-formatter test
extensions. No subsequent execution of those added native assertions has been
recorded. The later Windows presentation has user-confirmed manual evidence.
The recorded result also does not establish execution of other targets such as
``cubitsim_model_tests`` or ``calculix_result_unit_tests``.

Retained evidence summary
-------------------------

.. csv-table:: Units stage evidence as of 17 November 2026
   :file: verification/units-stage-20261117.csv
   :header-rows: 1
   :widths: 12 10 28 15 15 20

The CSV is a portable summary retained with the documentation. Detailed local
artifacts remain under
``tests/unit_verification_runs/20261116T132856Z_3f89f3cf/``, including
``report.json``, native logs, exported decks and saved models.
The ``tests`` directory is Git-ignored. The local journal and reports are not
distributed manual downloads; their absence from an installed component is
expected.

Automated Windows coverage
--------------------------

All three recorded native executables returned zero:

* ``calculix_domain_unit_tests``: domain conversion and persistence fixtures,
  failed conversion/read retention, Report views and related source assertions.
* ``calculix_unit_boundary_tests``: mechanical DOF mapping, source-to-geometry
  conversion, result quantity/axis metadata and retained run provenance.
* ``calculix_unit_presentation_tests``: dimensional draft conversion,
  affine temperatures, failure without partial relabelling, Apply blocking and
  recovery, current-unit settings adoption and frequency presentation.

These descriptions identify fixture coverage, not independent proof of every
possible combination. Read the corresponding project-owned test sources when
changing a boundary.

The live journal also passed:

* Export and save/reopen for ``SI``, ``N-mm-s``, ``kN-m-s``, ``N-mm-ms`` and
  ``kN-mm-ms``. Checks include geometry coordinates, elastic properties,
  translational force and rigid-body moment scaling.
* Logical rotation component 6 mapping to ROT NODE solver DOF 3, including
  a prescribed 30-degree rotation exported in radians.
* Expected rejection of a nonzero translational force on a ROT NODE.
  Printed Cubit errors belong to this negative test; they are not an
  unexpected campaign failure.
* Physical solves in ``SI``, ``N-mm-s`` and ``kN-m-s``. The measured Y
  displacement was ``-0.0471096 mm`` in each case. Projection was checked after
  changing the current workspace preset, using the recorded solver length
  basis and the declared Cubit geometry unit.

The live fixtures request FRD field output, without DAT history requests.
An empty DAT result file is therefore expected in these cases. The campaign
does not establish DAT history-output coverage.

User-confirmed Windows checks
-----------------------------

The user confirmed the following observations on 17 November 2026:

1. Unit labels and material values update consistently when display preferences
   change; stress, temperature and Poisson's ratio retain their physical meaning.
2. Valid numeric drafts convert between display units. Invalid drafts prevent
   partial conversion and block Apply until completed in their displayed basis.
3. Ordinary-node equation coefficients use inverse length units, while ROT NODE
   coefficients use inverse angle units and solver DOFs 1--3.
4. Frequency amplitude controls and modal-dynamic controls use the appropriate
   quantities. The modal time-period field becomes dimensionless when used as
   a steady-state relative error; time increments retain time units.
5. Scoped centrifugal drawing remains consistent when geometry and display
   lengths differ, including geometry in metres with display in millimetres
   and the reverse combination.

After these checks, the user also confirmed the compact label and heading
presentation. This is manual user evidence, not automated screenshot comparison.
No screenshot archive or binary revision was supplied for these confirmations.

Presentation and export contracts
---------------------------------

``CoreFrameworkUnits::display_unit`` defaults frequency presentation to ``Hz``
when no explicit frequency preference exists. Coherent solver units still
derive from the selected preset. An explicit ``1/ms`` display preference
remains valid and is not replaced by the default.

``UnitLabelFormat`` belongs to the shared ``cubitsim_model`` unit layer and uses
Qt Core rather than Qt Widgets. Its output is presentation text only:

* Flat products and quotients support integer and half-integer exponents.
  Negative powers form a grouped denominator; half powers can share a radical.
  For valid unit expressions the symbols denote positive scale factors.
* The supported examples include ``N/mm^2`` to ``N/mm²`` and
  ``N^0.5*m^0.5*s^-1`` to ``√(N·m)/s``.
* Unsupported, nested or arbitrary fractional-power expressions retain their
  original spelling. Explicit symbolic material-heading powers use the
  separate ``power`` entry point.
* Formatted strings must never be passed to the conversion parser, stored as
  physical-unit metadata or used to generate commands. Bound numeric drafts
  and raw unit expressions remain independent.

Material-table formatting retains original precision for unedited cells.
Dimensional solver string views use six decimal places in scientific notation;
canonical values are not rounded by exporting a detached view. Neither rule
rewrites opaque custom solver text.

See :doc:`../framework/units` and :doc:`../framework/materials` for user-facing
examples and commands.

Runtime and fixture blockers resolved
-------------------------------------

Initial startup failures and a later fixture error prevented completion:

* Qt's offscreen plugin was not configured consistently with the selected Qt
  installation. The launcher now discovers Qt paths from the configured
  installation; platform-plugin paths accompany ``QT_QPA_PLATFORM=offscreen``.
* Inherited Windows paths selected Cubit 2026.6 ``cubiti19.dll`` for a plugin
  built against Cubit 2026.11. The loader reported ``0xC0000139`` because the
  old DLL lacked ``CubitInterface::CIObserve::notify_iga_entity_modified``.
  Explicit configured Cubit ``bin`` and ``acis/code/bin`` directories now
  precede inherited paths. The Python host's location is not SDK provenance.
* The domain corruption fixture attempted to recreate existing HDF5 datasets.
  Independent round trips now use fresh files, and the malformed-unit fixture
  explicitly deletes the valid marker before creating its replacement.
  Rejection must still preserve the populated live model.

These resolutions are supported by the later passing Windows run. An earlier
startup failure, missing executable or partial run must not be counted as PASS.
Runtime search directories and each native result are written incrementally
to the local report.

Reproducing the recorded campaign
---------------------------------

The following commands are instructions, not an additional execution record.
Use a matching Cubit SDK, Qt installation and existing project dependencies.
From the repository root, enable and build the three recorded targets::

   cmake -S src -B build -DCUBITSIM_DOMAIN_UNIT_TESTS=ON
   cmake --build build --config Release --target calculix_domain_unit_tests calculix_unit_boundary_tests calculix_unit_presentation_tests -j 24

With the matching plugin loaded in Cubit, run the local developer journal::

   playback "tests/verify_calculix_units_all.jou"

Run playback from a directory where that relative path resolves, or provide
the journal's absolute path. The journal resets the current model; save current
work first. It does not build targets or save user defaults. Missing targets
remain incomplete and native failures stop the campaign before later cases.

For CTest-only execution in a configured Windows build, the registered cases
can be selected directly::

   ctest --test-dir build/CalculiX/Core/units/domain-unit-tests -C Release -R "^calculix_(domain_units|unit_boundaries)$" --output-on-failure
   ctest --test-dir build/CalculiX/GUI/common -C Release -R "^calculix_unit_presentation$" --output-on-failure

CTest runtime-path changes take effect after CMake configuration. Rebuilding
changed tests is required before their new source assertions can be evaluated.

Pending Linux verification
--------------------------

**Status: PENDING. No Linux PASS evidence is recorded for this units stage.**

A future Linux campaign must use matching native libraries and the offscreen Qt
plugin, run the native and live cases, and repeat manual checks 1--5 plus compact
labels and headings. Record distribution, Cubit/Qt/solver versions, plugin
revision or explicit absence of revision information, exit codes and retained
artifacts. Update the evidence summary only after those runs complete.

The Windows close-out does not remove this pending item or establish
cross-platform release approval.
