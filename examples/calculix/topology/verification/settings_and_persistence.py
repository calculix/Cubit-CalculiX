"""Batch 8a configuration checks inside Cubit; no solver or meshing is used.

This resets the current model and retains the last state on failure. Native
checks are not considered executed merely because this file passes syntax
inspection. CSV files contain actual command outcomes and queried settings.
The suite does not validate GUI interactions or level-set numerical behavior.
"""

import csv
from datetime import datetime, timezone
import math
from pathlib import Path
import platform
import sys
import traceback
from uuid import uuid4


LEVEL_SET_DEFAULTS = {
    "level_set_interface_width": 0.1,
    "level_set_regularization_length": 0.0,
    "level_set_move_limit": 0.1,
    "level_set_field_tolerance": 0.01,
    "level_set_seed_spacing": 0.0,
    "level_set_seed_radius": 0.0,
    "level_set_integration_tolerance": 0.0001,
    "level_set_integration_max_refinements": 4,
}
DEFAULTS = {
    "method": "density",
    "objective": "compliance",
    "volume_fraction": 0.3,
    "void_ratio": 0.001,
    "iterations": 100,
    "material_cap": 1024,
    "filter_radius": 0.0,
    "move_limit": 0.2,
    "adaptive_moves": "adaptive_moves_on",
    "minimum_move_limit": 0.0,
    "density_tolerance": 0.01,
    "compliance_tolerance": 0.001,
    "displacement_nodeset": 0,
    "displacement_component": "magnitude",
    **LEVEL_SET_DEFAULTS,
}
LEVEL_SET_VALUES = {
    "level_set_interface_width": 0.15,
    "level_set_regularization_length": 0.75,
    "level_set_move_limit": 0.05,
    "level_set_field_tolerance": 0.005,
    "level_set_seed_spacing": 3.0,
    "level_set_seed_radius": 0.4,
    "level_set_integration_tolerance": 0.0002,
    "level_set_integration_max_refinements": 3,
}


def require(condition, message):
    if not condition:
        raise AssertionError(message)


def matches(actual, expected):
    if isinstance(expected, (int, float)):
        value = float(actual)
        return math.isfinite(value) and math.isclose(value, expected, rel_tol=1.0e-13, abs_tol=1.0e-15)
    return actual == expected


def quote(value):
    text = str(value)
    require(not any(character in text for character in ('"', "\n", "\r")), "Unsupported command string")
    return '"' + text + '"'


def arguments(settings):
    parts = []
    for key, value in settings.items():
        if key == "name":
            parts.extend((key, quote(value)))
        elif key == "adaptive_moves":
            parts.append(str(value))
        else:
            parts.extend((key, str(value)))
    return " ".join(parts)


def run_suite(output_root=None, plugin_revision="not recorded"):
    """Reset Cubit, run checks, and retain CSV evidence in a new directory.

    A raised exception stops the suite without resetting away failure evidence.
    Expected command rejections may raise Cubit's SyntaxError wrapper; other
    exceptions always fail. The playback pause-on-error preference is restored.
    """
    import cubit
    import calculix_pythoninterface as wrapper

    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    output = (Path(output_root or Path.cwd() / "topology_verification") / (stamp + "_" + uuid4().hex[:8])).resolve()
    output.mkdir(parents=True, exist_ok=False)
    checks = []
    commands = []
    snapshots = []
    status = "FAILED"
    current_check = "interface capabilities"
    print("Topology configuration verification. Evidence: " + str(output))
    print("The current model will be reset. No solver will be started.")

    def write_csv(filename, columns, rows):
        with (output / filename).open("w", newline="", encoding="utf-8") as handle:
            writer = csv.writer(handle)
            writer.writerow(columns)
            writer.writerows(rows)

    def passed(name):
        checks.append((name, "PASS", ""))
        print("PASS: " + name)

    def settings(case_id, phase):
        rows = [list(row) for row in ccx.get_topology_settings(case_id)]
        require(all(len(row) == 2 for row in rows), "Invalid settings row layout")
        data = dict(rows)
        require(len(rows) == len(data), "Duplicate settings keys")
        require(set(data) == set(DEFAULTS) | {"name"}, "Missing or unexpected settings keys")
        for key, value in data.items():
            snapshots.append((phase, case_id, key, value))
        return data

    def command(text, succeeds=True):
        previous_pause = cubit.is_playback_paused_on_error()
        before_errors = cubit.get_error_count()
        returned = False
        detail = ""

        try:
            if not succeeds:
                cubit.set_playback_paused_on_error(False)

            try:
                returned = bool(cubit.cmd(text))
            except SyntaxError as error:
                detail = str(error)

            after_errors = cubit.get_error_count()
            ok = returned and after_errors <= before_errors
            require(ok == succeeds, "Unexpected command result: " + text)
        finally:
            cubit.set_playback_paused_on_error(previous_pause)
            commands.append((
                text,
                "success" if succeeds else "rejection",
                returned,
                before_errors,
                cubit.get_error_count(),
                detail
            ))

    def unchanged_except(before, after, keys):
        require(set(before) == set(after), "Settings keys changed")
        for key in before:
            if key not in keys:
                require(before[key] == after[key], "Unexpected change to " + key)

    def verify_defaults(case_id):
        data = settings(case_id, "defaults")
        require(data["name"] == "topology_" + str(case_id), "Unexpected default name")
        for key, expected in DEFAULTS.items():
            require(matches(data[key], expected), "Incorrect default for " + key)
        return data

    try:
        ccx = wrapper.CalculiXPythonInterface()
        for method in ("get_topology_settings", "get_topology_iteration_data", "get_topology_run_state"):
            require(callable(getattr(ccx, method, None)), "Loaded Python wrapper is missing " + method)
        for method in ("cmd", "get_error_count", "get_version", "is_playback_paused_on_error", "set_playback_paused_on_error"):
            require(callable(getattr(cubit, method, None)), "Cubit Python interface is missing " + method)
        metadata = [
            ("suite", "topology batch 8a: configuration and persistence"),
            ("started_utc", stamp),
            ("plugin_revision_supplied_by_user", plugin_revision),
            ("cubit_version", cubit.get_version()),
            ("python_version", sys.version),
            ("platform", platform.platform()),
            ("wrapper_path", getattr(wrapper, "__file__", "unavailable")),
            ("suite_path", str(Path(__file__).resolve())),
            ("scope", "unrun case settings; no numerical or GUI certification"),
        ]
        write_csv("metadata.csv", ("key", "value"), metadata)
        passed(current_check)

        current_check = "default creation"
        command("reset")
        command("sim version")
        # One unmeshed solid ensures a native model file can be written.
        command("create brick x 1 y 1 z 1")
        command("ccx create topology 1")
        command("ccx create topology 2")
        verify_defaults(1)
        verify_defaults(2)
        passed(current_check)

        current_check = "create accepts all level-set options"
        command("ccx create topology 3 method level_set " + arguments(LEVEL_SET_VALUES))
        created = settings(3, "explicit_create")
        for key, expected in {**LEVEL_SET_VALUES, "method": "level_set"}.items():
            require(matches(created[key], expected), "Create did not store " + key)
        passed(current_check)

        command("ccx modify topology 2 method level_set")
        for key, value in LEVEL_SET_VALUES.items():
            current_check = "isolated modification: " + key
            before = settings(2, "before_" + key)
            command("ccx modify topology 2 " + arguments({key: value}))
            after = settings(2, "after_" + key)
            unchanged_except(before, after, {key})
            require(matches(after[key], value), "Modification did not store " + key)
            passed(current_check)

        current_check = "refinement boundary values"
        for refinements in (1, 5):
            command("ccx modify topology 2 level_set_integration_max_refinements " + str(refinements))
            require(int(settings(2, "refinement_boundary")["level_set_integration_max_refinements"]) == refinements,
                    "Valid refinement boundary was rejected or changed")
        command("ccx modify topology 2 level_set_integration_max_refinements 3")
        passed(current_check)

        current_check = "automatic length settings"
        automatic = {key: 0 for key in ("level_set_regularization_length", "level_set_seed_spacing", "level_set_seed_radius")}
        before = settings(2, "before_automatic")
        command("ccx modify topology 2 " + arguments(automatic))
        after = settings(2, "automatic")
        unchanged_except(before, after, set(automatic))
        for key in automatic:
            require(float(after[key]) == 0.0, "Automatic value was resolved during configuration")
        command("ccx modify topology 2 " + arguments(LEVEL_SET_VALUES))
        passed(current_check)

        current_check = "unrelated edit preserves method settings"
        before = settings(2, "before_name")
        command('ccx modify topology 2 name "level set verification"')
        after = settings(2, "after_name")
        unchanged_except(before, after, {"name"})
        require(after["name"] == "level set verification", "Name not stored")
        passed(current_check)

        current_check = "density and level-set settings survive method switches"
        command("ccx modify topology 2 filter_radius 2 move_limit 0.15 minimum_move_limit 0.03 adaptive_moves_off density_tolerance 0.02")
        before = settings(2, "before_method_switch")
        command("ccx modify topology 2 method density")
        density = settings(2, "density_method")
        unchanged_except(before, density, {"method"})
        require(density["method"] == "density", "Density method not stored")
        command("ccx modify topology 2 method level_set")
        require(settings(2, "level_set_restored") == before, "Method switch lost settings")
        passed(current_check)

        invalid = [
            ("level_set_interface_width", 0), ("level_set_interface_width", 1.1),
            ("level_set_regularization_length", -1),
            ("level_set_move_limit", 0), ("level_set_move_limit", 2.1),
            ("level_set_field_tolerance", 0), ("level_set_field_tolerance", 2.1),
            ("level_set_seed_spacing", -1), ("level_set_seed_radius", -1),
            ("level_set_seed_radius", 1.5),
            ("level_set_integration_tolerance", 0), ("level_set_integration_tolerance", 1),
            ("level_set_integration_max_refinements", 0),
            ("level_set_integration_max_refinements", 6),
            ("level_set_integration_max_refinements", 2.5),
        ]
        for key, value in invalid:
            current_check = "reject " + key + " " + str(value)
            before = settings(2, "before_invalid")
            command('ccx modify topology 2 name "must_not_commit" ' + arguments({key: value}), succeeds=False)
            require(settings(2, "after_invalid") == before, "Rejected edit changed stored settings")
            passed(current_check)

        current_check = "invalid level-set settings rejected on density cases"
        before = settings(1, "density_before_invalid")
        command("ccx modify topology 1 level_set_interface_width 0", succeeds=False)
        require(settings(1, "density_after_invalid") == before, "Invalid dormant settings were stored")
        passed(current_check)

        current_check = "invalid creation does not reserve a case ID"
        command("ccx create topology 4 method level_set level_set_interface_width 0", succeeds=False)
        command("ccx create topology 4")
        verify_defaults(4)
        command("ccx delete topology 4")
        passed(current_check)

        current_check = "public level-set run rejected without changing case records"
        before = settings(2, "before_run_rejection")
        state = ccx.get_topology_run_state(2)
        rows = [list(row) for row in ccx.get_topology_iteration_data(2)]
        command("ccx run topology 2", succeeds=False)
        require(settings(2, "after_run_rejection") == before, "Run rejection changed settings")
        require(ccx.get_topology_run_state(2) == state, "Run rejection changed run state")
        require([list(row) for row in ccx.get_topology_iteration_data(2)] == rows, "Run rejection created iteration associations")
        passed(current_check)

        current_check = "delete and recreate uses fresh defaults"
        command("ccx delete topology 3")
        command("ccx create topology 3")
        verify_defaults(3)
        command("ccx delete topology 3")
        passed(current_check)

        current_check = "save reset reopen retains both cases"
        command('ccx modify topology 1 name "density verification" volume_fraction 0.55 iterations 12 material_cap 128 compliance_tolerance 0.002')
        saved = {case_id: settings(case_id, "before_save") for case_id in (1, 2)}
        model = output / "settings_and_persistence.cub5"
        command("save as " + quote(model.as_posix()))
        require(model.is_file() and model.stat().st_size > 0, "Model file was not created")
        command("reset")
        command("open " + quote(model.as_posix()))
        for case_id, expected in saved.items():
            require(settings(case_id, "after_reopen") == expected, "Save/load changed case " + str(case_id))
            require(ccx.get_topology_run_state(case_id) == "not_run", "Unexpected restored run state")
            require(not list(ccx.get_topology_iteration_data(case_id)), "Unrun case gained iterations")
        passed(current_check)

        status = "PASSED"
        print("AUTOMATED CONFIGURATION CHECKS PASSED. GUI and numerical checks remain unexecuted.")
        print("Current model contains density case 1 and level-set case 2 for manual GUI checks.")
        return output
    except Exception as error:
        checks.append((current_check, "FAIL", str(error)))
        (output / "failure.txt").write_text(traceback.format_exc(), encoding="utf-8")
        print("STOPPED: " + current_check + ". Current model and evidence are retained.")
        raise
    finally:
        write_csv("checks.csv", ("check", "status", "detail"), checks)
        write_csv("commands.csv", ("command", "expected", "returned_success", "errors_before", "errors_after", "detail"), commands)
        write_csv("settings.csv", ("phase", "topology_id", "keyword", "value"), snapshots)
        write_csv("summary.csv", ("key", "value"), (
            ("automated_configuration", status),
            ("manual_gui", "NOT EXECUTED"),
            ("historical_iteration_persistence", "NOT EXECUTED"),
            ("numerical_verification", "NOT EXECUTED"),
            ("plugin_revision_supplied_by_user", plugin_revision),
        ))


if __name__ == "__main__":
    run_suite(plugin_revision=globals().get("PLUGIN_REVISION", "not recorded"))
