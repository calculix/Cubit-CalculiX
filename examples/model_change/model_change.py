#!python
#!python
#!python
#!python
#!python
from numpy import cos,sin,arccos
import numpy as np
import cubit

cubit.init()
cubit.cmd("reset")
cubit.cmd("undo off")


def check_if_enclosed(vid1,vid2):
 v1 = cubit.volume(vid1)
 v2 = cubit.volume(vid2)
 b_box1 = v1.bounding_box()
 b_box2 = v2.bounding_box()
 for i in range(3):
  if b_box1[i] > b_box2[i]:
   return False
  if b_box1[i+3] < b_box2[i+3]:
   return False
 return True

def split_bodies():
 body_list = cubit.get_entities("body")
 for b in body_list:
  if cubit.is_multi_volume(b): 	 
   cubit.cmd(f"split body {b}")
 return True

def parametric_circle(t,xc,yc,R):
 x = xc + R*cos(t)
 y = yc + R*sin(t)
 return x,y

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

N = 25
R = plate_y*1/3
xc = 0
yc = 0

arc_T = np.linspace(3.14159265359/2, 3.14159265359*3/2, N)
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
for i in range(N):
 cubit.cmd("Volume 2 copy move x " + str(X[i]) + " y " + str(Y[i]) + " z " + str(0))
 vid = cubit.get_last_id("volume")
 if i!=0:
  overlapping_volumes = cubit.get_overlapping_volumes_at_volume(vid,cubit.get_entities("volume"))
  for ov in overlapping_volumes:
   if ov!=1 and ov!=2:
    split_bodies()
    if check_if_enclosed(vid,ov):
     cubit.cmd(f"delete volume {ov}")
    else:
     cubit.cmd(f"subtract volume {vid} from volume {ov} keep_tool")

cubit.cmd("delete vol 2")

cubit.cmd("imprint vol all")
cubit.cmd("merge vol all")
cubit.cmd("vol all except 1 size 2.5")
cubit.cmd("vol 1 size 10")
cubit.cmd("surface all with z_coord = "+str(plate_z + cube_z)+" scheme pave")
cubit.cmd("mesh surface all with z_coord = "+ str(plate_z + cube_z))
cubit.cmd("mesh vol all except 1")
cubit.cmd("surface all in volume 1 with z_coord >= " + str(plate_z) +" scheme pave")
cubit.cmd("mesh surface all with z_coord >= "+str(plate_z))
cubit.cmd("mesh vol 1")
cubit.cmd("compress")

volume_list = cubit.get_entities("volume")
for v in volume_list:
 cubit.cmd(f"block {v} add vol {v}")
 cubit.cmd(f"nodeset {v} add vol {v}")
 cubit.cmd(f"sideset {v} add surface all in vol 1 to {v} with is_merged = 0")


#material
cubit.cmd('create material "solid" property_group "CalculiX-FEA"')
cubit.cmd('modify material "solid" scalar_properties "CCX_ELASTIC_ISO_USE_CARD" 1')
cubit.cmd('modify material "solid" scalar_properties "CCX_PLASTIC_ISO_USE_CARD" 1')
cubit.cmd('modify material "solid" scalar_properties "CCX_DENSITY_USE_CARD" 1')
cubit.cmd('modify material "solid" matrix_property "CCX_DENSITY_DENSITY" 8 0 ')
cubit.cmd('modify material "solid" scalar_properties "CCX_SPECIFIC_HEAT_USE_CARD" 1')
cubit.cmd('modify material "solid" matrix_property "CCX_SPECIFIC_HEAT_SPECIFIC_HEAT" 420 0 ')
cubit.cmd('modify material "solid" scalar_properties "CCX_EXPANSION_ISO_USE_CARD" 1')
cubit.cmd('modify material "solid" scalar_properties "CCX_CONDUCTIVITY_USE_CARD" 1')
cubit.cmd('modify material "solid" scalar_properties "CCX_CONDUCTIVITY_ISO_USE_CARD" 1')
cubit.cmd('modify material "solid" matrix_property "CCX_CONDUCTIVITY_ISO_K_TEMPERATURE" 50 0 ')
cubit.cmd('modify material "solid" scalar_properties "CCX_ELASTIC_USE_CARD" 1')
cubit.cmd('modify material "solid" matrix_property "CCX_ELASTIC_ISO_MODULUS_VS_POISSON_VS_TEMPERATURE" 210000 0.3 0') 


#section
cubit.cmd('ccx create section solid block all material 1')

#heatflux and initial temperature
volume_list = cubit.get_entities("volume")
for v in volume_list:
 cubit.cmd(f"create heatflux  on sideset {v} value -15")
 #cubit.cmd(f"ccx modify heatflux {v} op new")
 cubit.cmd(f"create temperature on volume {v} value 2000")
 cubit.cmd(f"ccx modify temperature {v} op new")

cubit.cmd("modify temperature 1 value 100")

# field outputs
cubit.cmd('ccx create fieldoutput name "fo_node" node')
cubit.cmd('ccx modify fieldoutput 1 node frequency 10')
cubit.cmd('ccx modify fieldoutput 1 node key_on NT')
cubit.cmd('ccx modify fieldoutput 1 node key_off CP U DEPF DEPT DTF HCRI KEQ MACH MAXU MF PNT POT PRF PS PSF PT PTF PU RF RFL SEN TS TSF TT TTF TURB V VF ')
cubit.cmd('ccx create fieldoutput name "fo_element" element')
cubit.cmd('ccx modify fieldoutput 2 element frequency 10')
cubit.cmd('ccx modify fieldoutput 2 element key_on HFL')
cubit.cmd('ccx modify fieldoutput 2 element key_off CEEQ S E ECD EMFB EMFE ENER ERR HER HFLF MAXE MAXS ME PEEQ PHS SF SMID SNEG SPOS SVF SDV THE ZZS ')

# initial conditions
cubit.cmd("ccx create initialcondition temperature")
cubit.cmd(f"ccx modify initialcondition {1} temperature bc_id {1}")

cubit.cmd("ccx create step name 'dummy_step' static")
cubit.cmd("ccx step 1 add fieldoutput 1 2")
cubit.cmd("ccx create step name 'deacivate_step' static")
cubit.cmd("ccx step 2 add fieldoutput 1 2")
cubit.cmd("ccx modify step 2 parameter nlgeom_yes")
cubit.cmd("ccx create customline name 'step_2_top' after step_begin item_id 2 cline '*MODEL CHANGE,TYPE=ELEMENT,REMOVE'")
volume_list = cubit.get_entities("volume")
for v in volume_list:
 if v >1:
  cubit.cmd(f"ccx create customline name '{v}' after step_begin item_id 2 cline 'Block_{v}'")

cs = 2
volume_list = cubit.get_entities("volume")
for v in volume_list:
# if v > 1 and v < 3:
 if v > 1:
  cs = cs + 1
  cubit.cmd(f"ccx create step name 'heat_{cs}' heattransfer")
  cubit.cmd(f"ccx modify step {cs} heattransfer initialtimeincrement 10 timeperiodofstep 10 minimumtimeincrement 1e-5 maximumtimeincrement 10")
  cubit.cmd(f"ccx step {cs} add load heatflux {v}")
  cubit.cmd(f"ccx step {cs} add bc temperature {v}")
  cubit.cmd(f"ccx step {cs} add fieldoutput 1 2")
  cubit.cmd(f"ccx create customline name 'step_{cs}_top' after step_begin item_id {cs} cline '*MODEL CHANGE,TYPE=ELEMENT,ADD'")
  cubit.cmd(f"ccx create customline name '{v}' after step_begin item_id {cs} cline 'Block_{v}'")
  cs = cs + 1
  cubit.cmd(f"ccx create step name 'heat_{cs}' heattransfer")
  cubit.cmd(f"ccx modify step {cs} heattransfer initialtimeincrement 10 timeperiodofstep 100 minimumtimeincrement 1e-5 maximumtimeincrement 10")
  cubit.cmd(f"ccx step {cs} add load heatflux {v}")
  cubit.cmd(f"ccx step {cs} add fieldoutput 1 2")
  totaltimeatstart = timeperiodofstep
  timeperiodofstep = timeperiodofstep + 10

cubit.cmd("ccx create job name 'model_change'")

print("ccx run job 1 no_conversion")
command = ("ccx result convert job 1 block ")
for v in volume_list:
 command = command + str(v) + " "
command = command + " partial"
print(command)


