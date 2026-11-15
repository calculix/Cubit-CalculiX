import os
import subprocess
from pathlib import Path

root = Path(__file__).resolve().parents[2]
output = root / "startup-diagnostics" / "current-plain"
output.mkdir(parents=True, exist_ok=True)
env = os.environ.copy()
env["CUBITSIM_STARTUP_TRACE"] = str(output / "startup.log")
env["CUBIT_PLUGIN_DIR"] = str(root / "build" / "Release")
env.pop("VIRTUAL_ENV", None)
env["PATH"] = "C:/Program Files/Coreform Cubit 2026.11/acis/code/bin;" + env.get("PATH", "")
startup = subprocess.STARTUPINFO()
startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
startup.wShowWindow = 0
with (output / "stdout.log").open("wb") as stdout, (output / "stderr.log").open("wb") as stderr:
    process = subprocess.Popen(["C:/Program Files/Coreform Cubit 2026.11/bin/clarox.exe", "-prompt", "Cubit"], env=env, cwd=output, stdout=stdout, stderr=stderr, startupinfo=startup)
    print("Diagnostic PID:", process.pid, flush=True)
    try:
        print("Exit:", process.wait(timeout=20), flush=True)
    except subprocess.TimeoutExpired:
        print("Still running after 20 seconds; closing only the diagnostic process.", flush=True)
        process.terminate()
        process.wait(timeout=10)
for path in output.glob("*.log"):
    print(path.name, path.read_text(encoding="utf-8", errors="replace")[-6000:])
