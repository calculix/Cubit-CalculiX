#!python
from numpy import cos,sin,arccos
import numpy as np

def parametric_circle(t,xc,yc,R):
    x = xc + R*cos(t)
    y = yc + R*sin(t)
    return x,y

cubit.init()
cubit.cmd("reset")

plate_x = 200
plate_y = 100
plate_z = 5

cube_x = 5
cube_y = 5
cube_z = 5

cubit.cmd("create brick x " + str(plate_x) + " y " + str(plate_y) + " z " + str(plate_z))
cubit.cmd("create brick x " + str(cube_x) + " y " + str(cube_y) + " z " + str(cube_z))
cubit.cmd("move Volume 1 location " + str(plate_x/2) + " " + str(plate_y/2) + " " + str(plate_z/2))
cubit.cmd("move Volume 2 location " + str(cube_x/2 + plate_x/3 - plate_x/8) + " " + str(plate_y/2) + " " + str(plate_z + cube_z/2))

N = 30
R = plate_y*1/3
xc = 0
yc = 0

arc_T = np.linspace(3.14/2, 3.14*3/2, N)
X,Y = parametric_circle(arc_T, xc, yc, R)

vid = 0
for i in range(N):
 cubit.cmd("Volume 2 copy move x " + str(X[i]) + " y " + str(Y[i]) + " z " + str(0))
 vid_last = vid
 vid = cubit.get_last_id( "volume")
 if i!=0:
  cubit.cmd(f"subtract volume {vid} from volume {vid_last} keep_tool")
 
 
cubit.cmd("move Volume 2 location " + str(cube_x/2 + plate_x*2/3 - plate_x/8) + " " + str(plate_y/2) + " " + str(plate_z + cube_z/2))
vid=0
for i in range(N):
 cubit.cmd("Volume 2 copy move x " + str(X[i]) + " y " + str(Y[i]) + " z " + str(0))
 vid_last = vid
 vid = cubit.get_last_id( "volume")
 if i!=0:
  cubit.cmd(f"subtract volume {vid} from volume {vid_last} keep_tool")

vid=0
cubit.cmd("move Volume 2 location " + str(cube_x/2 + plate_x*4/5) + " " + str(plate_y/2) + " " + str(plate_z + cube_z/2))
X = np.linspace(-plate_y*1/5, plate_y*1/5, N)
Y = np.linspace(-plate_y*1/3, plate_y*1/3, N)
for i in range(N):
 cubit.cmd("Volume 2 copy move x " + str(X[i]) + " y " + str(Y[i]) + " z " + str(0))
 vid_last = vid
 vid = cubit.get_last_id( "volume")
 if i!=0:
  cubit.cmd(f"subtract volume {vid} from volume {vid_last} keep_tool")

X = np.linspace(-plate_y*1/5, plate_y*1/5, N)
Y = np.linspace(plate_y*1/3, -plate_y*1/3, N)
for i in range(N-14):
 cubit.cmd("Volume 2 copy move x " + str(X[i]) + " y " + str(Y[i]) + " z " + str(0))
 vid = cubit.get_last_id("volume")
 if i!=0:
  overlapping_volumes = cubit.get_overlapping_volumes_at_volume(vid,cubit.get_entities("volume"))
  for ov in overlapping_volumes:
   if ov!=1 and ov!=2:
    cubit.cmd(f"subtract volume {vid} from volume {ov} keep_tool")
    if (cubit.was_last_cmd_undoable()):
     vid_created = cubit.get_last_id("volume")
     #cubit.cmd(f"delete volume {vid_created}")
     print(cubit.was_last_cmd_undoable())

#cubit.cmd("delete vol 2")
#volume_list = cubit.get_entities("volume")

#for v in volume_list:
# cubit.cmd("delete vol " + str(v))

cubit.cmd("list hex all")
#cubit.cmd("imprint vol all")
#cubit.cmd("merge vol all")