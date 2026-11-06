Postprocessor verification
==========================

Status and scope
----------------

This is an acceptance checklist, not executed test evidence. A user-reported
Release build with MSVC 14.44.35207 failed at the original session-swap assertion
in ``PostprocessorCoreSources.cpp``. The installed MSVC headers show a potentially
allocating ``std::map`` move constructor; generic session swapping therefore did
not meet the asserted nonthrowing contract. Publication now uses a conditional
``noexcept`` member-wise swap and checks that exact operation at compile time.
The user subsequently confirmed a successful rebuild after the fix. Building and compiling by the agent remain
prohibited until explicitly authorized. Documentation rendering has not been run
either. Read :doc:`postprocessor` for ownership, method contracts and current limits.

The subsequent workspace patch has not been compiled. After its build, execute
the failure-preservation and refresh tests below; passing the compiler assertion
alone is not evidence that source lifecycle or journal replay works correctly.

Core lifecycle checks after build authorization
-----------------------------------------------

Use a small fake result provider for deterministic success/failure cases before
testing installed solver data. Tests should exercise externally observable
behavior rather than repeat implementation details.

* Default session: exactly one view/pane, no sources and no active source.
* Open two unrelated sources: both retained, unique IDs, last source selected,
  existing layout unchanged, arrays retained through owning shared handles.
* Names: provider default, explicit override, blank rejection, exact case-sensitive
  lookup, duplicate rejection without replacing data. Explicit duplicates should
  not call the provider; suggested-name duplicates may call it but cannot commit.
* Publication failures: null/empty data, wrong reference category, mismatched job,
  application/project/token, wrong file provider or unresolved published paths.
* Provider failure/exception: old state, data, selection and ID allocation remain
  unchanged. Verify the busy guard releases so a later operation succeeds.
* Reentry: have a provider callback attempt open, refresh, selection and closure.
  Mutations must fail; read queries must see the old committed state.
* Refresh: keep ID/name/selection/layout, replace provenance/data, advance revision,
  and keep a caller's old data handle usable. Failure preserves the old revision.
* Selection and closure: unknown IDs fail; selecting clears representation context;
  closing active source clears selection; close-all retains view/camera/layout.
  Direct representation cleanup needs a fixture once representation creation exists.
* Counter boundaries: source IDs and revisions do not wrap. Boundary fixtures need
  controlled state construction when persistence/test access is introduced.

Provider and integration checks
-------------------------------

* Framework: optional providers absent by default; result-only modules register
  without command/GUI providers. Empty/duplicate prefixes fail with cleanup.
* Lookup: ``ccx`` and ``calculix`` resolve the same provider while initialized;
  lookup before initialization/after shutdown returns null. Published data survives
  provider teardown; callers must not retain borrowed provider pointers.
* CalculiX job: verify name/provenance, node/element IDs, groups, fields, components,
  frames/tables and representative numerical values against the existing converter.
* Reject missing/running jobs, active conversions, project-qualified requests,
  unsupported independent files and unavailable explicit snapshot tokens.
* Refresh a completed job explicitly; confirm that old Postprocessor snapshots
  survive provider cache replacement and that cancellation cannot partially commit.

Command and module checks after build authorization
---------------------------------------------------

The following handlers exist in source but have not been executed natively:
``post open <provider> job <id> [name "..."]``, ``post list``,
``post select "..."``, ``post close "..."`` and ``post close all``.

* Load the plugin and verify all four command keys register once alongside the
  existing CalculiX and Framework commands. The Postprocessor workspace tab appears
  in GUI mode; headless startup exposes commands without creating widgets.
* Verify parser extraction for ``ccx``, integer IDs, optional quoted names, spaces
  in names and a source named ``"all"`` versus the bulk keyword ``all``.
* Open an existing completed job, list the exact job name and active marker,
  select it and close it. Repeat with an explicit name and a second source.
* Unknown prefixes, nonpositive/missing/running jobs, blank/duplicate names and
  missing source names must return command failure with an appropriate error.
* Confirm two loaded sources retain independent snapshots without creating views.
* Record and replay the commands through Cubit's normal journal path using a
  controlled model/result fixture; check source names, count, selection and values.
  Changing result files is outside historical-replay guarantees.
* Retain a command object across module shutdown and reinitialization. Its weak
  application reference must expire; it must not target the new application.
* Close all twice and list: both closures succeed, the empty message is shown,
  and default view/layout definitions remain. No solver files/jobs are deleted.

Workspace checks after build authorization
------------------------------------------

* Reconfigure using the same Qt6 and Cubit 2026.11 installation. An older data-only
  VTK adapter should re-prepare rendering/Qt headers and import libraries. Confirm
  no VTK runtime DLL is built/copied and the original installation is untouched.
* Inspect the header below the title and above the menu. Tab switching must submit
  ``sim workspace postprocessor`` through the normal command window/journal path.
* Switch repeatedly among all three workspaces. Cubit/CalculiX must retain their
  original viewport and camera; Postprocessor must show its independent empty one.
* Verify left source/right properties docks, native navigation/toolbars hidden,
  command/error/history below only the middle column and unchanged status/progress.
* Switch away and restore previous command-dock properties and native toolbar
  visibility. Saved layouts, failed switches, GUI detach/re-attach and application
  shutdown must restore native graphics without deleting or duplicating it.
* Open/select/close sources through both typed commands and tree/menu/toolbar
  actions. Refresh must not submit extra selection commands. Verify quoted names,
  Unicode, source named ``all``, and safe rejection of control/quote injection.
* Check native command/close icons in both themes and standard-icon fallback when
  resources are absent. Confirm tooltips and enabled states match selection.
* Resize, minimize/restore and change displays/DPI. Rendering starts only with a
  valid Qt OpenGL context. Shell input remains blocked; no camera mutation occurs.
* Retain a source across GUI detach/re-attach. Stop observer callbacks before widget
  destruction and verify no queued refresh targets a deleted workspace.
* Record the first successful workspace build separately from native GUI, GL,
  source-value and journal verification; none has been performed by the agent.

Deferred feature checks
-----------------------

Native command journaling/replay remains unverified. Automatic display creation, VTU/VTPC/VTT import,
independent FRD/DAT loading, viewport navigation commands, geometry picking,
comparison links, derived features and save/reopen require their corresponding
implementations. Do not report these as verified from Core source inspection.
