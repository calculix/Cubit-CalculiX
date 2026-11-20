"""Run the CCX moving-joint reaction checks in a temporary directory."""
import argparse
import math
from pathlib import Path
import re
import subprocess
import tempfile


def cross(a, b):
    return [a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]]


def rows(block):
    return {int(row[0]): tuple(map(float, row[1:])) for line in block.splitlines()
            if len(row := line.split()) == 4 and row[0].isdigit()}


def check(deck, solver, directory):
    path = directory / deck.name
    path.write_bytes(deck.read_bytes())
    subprocess.run([solver, "-i", path.stem], cwd=directory, check=True,
                   stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=120)
    dat = path.with_suffix(".dat").read_text()
    pattern = (r"displacements .*?time\s+(\S+)\s*\n(.*?)\n\s*forces .*?time\s+(\S+)"
               r"\s*\n(.*?)(?=\n\s*INCREMENT|\Z)")
    peak_force = peak_moment = peak_axis = peak_gap = 0.
    times = []
    for time, displacements, force_time, forces in re.findall(pattern, dat, re.S):
        t = float(time)
        if t != float(force_time) or (times and t <= times[-1]):
            raise ValueError("Unexpected DAT increment times")
        times.append(t)
        u, rf = rows(displacements), rows(forces)
        residual = [a+b+t*load for a,b,load in zip(rf[90],rf[91],(0,15,3))]
        peak_force = max(peak_force, math.hypot(*residual))
        a, b = ([u[m][i]-u[o][i]+int(i == 0) for i in range(3)]
                for m,o in ((14,11),(13,9)))
        peak_axis = max(peak_axis, math.dist(a,b))
        lever = [x-y for x,y in zip(u[11],u[9])]
        travel = sum(x*y for x,y in zip(b,lever))
        projected = [x*travel for x in b] if "cylindrical" in deck.stem else [0.,0.,0.]
        peak_gap = max(peak_gap, math.dist(lever,projected))
        # External force at the translating RP includes its three foundation springs.
        f = [load*t-100*x for load,x in zip((0,15,0),u[11])]
        moment = cross(lever,f)
        offset = [x-y+initial for x,y,initial in zip(u[7],u[9],(.5,1.,0.))]
        spring_moment = cross(offset,(0.,0.,3*t-10*u[7][2]))
        mx,my,mz = [x+y for x,y in zip(moment,spring_moment)]
        # ROT generalized forces are conjugate to the rotation vector.
        # For this prescribed Z rotation, transform the spatial moment by J^T.
        angle = u[10][2]
        sinc = math.sin(angle)/angle
        cosc = (1-math.cos(angle))/angle
        generalized = (sinc*mx+cosc*my,sinc*my-cosc*mx,mz)
        peak_moment = max(peak_moment, math.hypot(*(x+y for x,y in zip(rf[10],generalized))))
    if not times or times[-1] != 1.:
        raise ValueError(f"{deck.name}: missing final increment")
    twist = math.degrees(math.asin(u[7][2]-u[5][2]))
    expected_travel = .2982051 if "cylindrical" in deck.stem else 0.
    if abs(twist-11.5428) > .001 or abs(travel-expected_travel) > 2.e-6:
        raise ValueError(f"{deck.name}: final twist {twist} degrees, travel {travel} mm")
    if peak_force > 1.e-5 or peak_moment > .001 or max(peak_axis,peak_gap) > 2.e-6:
        raise ValueError(f"{deck.name}: {len(times)} increments, force {peak_force}, "
                         f"moment {peak_moment}, axis {peak_axis}, gap {peak_gap}")
    print(f"{deck.stem}: {len(times)} increments; peak force {peak_force:.3g} N, "
          f"peak generalized moment {peak_moment:.3g} N mm")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("solver", type=Path)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory() as work:
        for joint in ("revolute","cylindrical"):
            check(Path(__file__).with_name(f"balance_{joint}.inp"), args.solver.resolve(), Path(work))
