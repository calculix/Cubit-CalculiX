Connections
===========

Connections contains reference points (RPs), rigid bodies, distributing attachments, joints, point masses and springs. The
feature uses Cubit geometry IDs for placement and membership. Pick geometry in
Cubit, open a Connections creation panel and choose **Use selected geometry**.
The ID fields also accept explicit journal IDs. Toggle the Connections drawing
button or issue ``ccx display connections on`` to show reference locations,
rigid attachments and joint axes. Hover over an RP in the tree to see its uses.

Placement and attachment
------------------------

An RP may use a vertex, the Cubit center of a curve, surface or volume, or XYZ
coordinates in the model's geometry units. Geometry-based placements resolve
again at export. A rigid body takes nodes from a curve, surface, volume, block
or nodeset. For a curve, surface or volume choose **Create reference point at
center** to create both objects with one command.

.. code-block:: text

   ccx create rigid body 1 name "Flange" surface 18 center
   ccx create rp 5 name "Pin center" center curve 12
   ccx create rigid body 2 name "Pin attachment" surface 42 rp 5

The exporter writes distinct REF and ROT node IDs and a private membership
set. Generated control nodes stay outside ALLNODES. Use a non-collinear
attachment region. A meshed vertex RP reuses its exported node for REF. A force or prescribed
translation on a nodeset containing that vertex acts on REF. The exporter
creates ROT automatically. An explicit rotation vertex is also available for
a separately named nodeset carrying moments or prescribed rotations.

.. code-block:: text

   ccx create rp 1 name "Control" vertex 5 rotation_vertex 6
   ccx create rigid body 1 name "Support" nodeset 1 rp 1

Forces and moments at a reference point
---------------------------------------

In **Create Force** or **Modify Force**, choose **Reference point** and select
its ID and name. The RP tree action **Apply force or moment...** opens the same
panel with that RP selected. **Nodeset** remains available for mesh loads.
You can assign a load before meshing. An RP used by an exported load must
have a rigid body or a distributing attachment. Force and moment components use the global axes
and the active force and moment units.

.. code-block:: text

   ccx create force rp 1 dof_2 -100 dof_6 25 name "Bearing load"
   ccx step 1 add load force 1
   ccx modify force 1 rp 2
   ccx modify force 1 nodeset 3

Omitted components default to zero on creation and keep their values on
modification. For a rigid body the exporter applies force components 1–3 at REF and moment
components 4–6 at ROT, using solver DOFs 1–3 on each node. Step assignment,
amplitude and time delay work the same way for both target types.

Distributing an RP load over a deformable face
----------------------------------------------

Under **Connections**, create a **Distributing attachment** or right-click an
RP and choose **Attach to deformable surface...**. Select a planar Cubit
surface or a face-based sideset on exported solid elements. A CAD surface
can create its own RP at the surface center. For a sideset, select an existing
RP. The faces must be coplanar and consistently oriented. An interior CAD
face shared by two solids needs a sideset identifying the loaded side.

All three force components are available. Select the moment axes to transfer.
The default enables all three. Use the existing **Create Force** panel and
assign the load to a step. CCX distributes the applied force and moment over
the face while its nodes can deform. An offset RP also gives the applied force
a lever arm. Each RP has one attachment and its face nodes must be distinct
from those of other distributing or rigid attachments.

.. code-block:: text

   ccx create distributing attachment 1 name "Flange" surface 18 center mean_displacement
   ccx create force rp 1 dof_2 -100 dof_6 25 name "Flange load"
   ccx step 1 add load force 1

In this example the new center receives RP ID 1 in an empty Connections tree.
For an existing RP and a sideset, use for example
``ccx create distributing attachment 2 name "Pad" sideset 8 rp 5 moments 4``.
The moment mask adds X=1, Y=2 and Z=4. Zero transfers forces only.

In the Postprocessor, open **Reference points** and choose the RP:

* **Prescribed load** shows the nominal force and moment components per
  assigned load and step, with exported amplitude parameters when present.
* **Surface resultant about RP** shows the DAT section force and the moment
  shifted from the global origin to the exported RP position. This is an
  integrated stress result and depends on mesh resolution.
* **Mean surface displacement** uses the motion of the actual face nodes.
  Weights integrate a piecewise-linear interpolation over triangles from each
  boundary edge to the face center, including midside nodes.

Surface resultants are enabled by default. Enable mean surface displacement
on creation to request nodal displacement output. Use the RP's **Surface
history output** menu to assign mechanical steps. The export uses a dedicated
solver load node and records the RP position and selected history weights
with the run. See ``examples/calculix/connections/distributed_flange.jou``.

Point masses and linear springs
-------------------------------

Choose **Point mass** or **Spring** in Connections, or use **Add point mass...**
or **Add spring...** on an RP to preselect it. Pick a vertex in Cubit and use
**Use selected vertex**, or select a reference point by name. The panel shows
the workspace display unit. In journals, use an explicit unit to replay a
value independently of display settings.

.. code-block:: text

   ccx create point mass 1 name "Motor" rp 3 mass 2 unit "kg"
   ccx create spring 1 name "Return spring" from rp 3 to vertex 12 stiffness 200 unit "N/mm"
   ccx create spring 2 name "Y support" from rp 3 to ground direction y stiffness 50 unit "N/mm"
   ccx modify point mass 1 mass 2.5 unit "kg"
   ccx modify spring 2 stiffness 100 unit "N/mm"
   ccx spring 1 history step 2
   ccx point mass 1 history off
   ccx delete spring 2

A point mass contributes translational inertia. An axial spring connects two
separated endpoints along their line of centers. A ground spring acts along a
selected global X, Y or Z direction. Enter positive mass and stiffness values.
A meshed vertex resolves to a structural node. A rigid-body RP uses its
translational REF node and can carry a mass, force and multiple springs.
A standalone vertex-backed RP also works when its vertex resolves to an
exported structural node. A geometry-center or XYZ RP needs a physical rigid
attachment. A distributing attachment's control node serves to apply loads.
A vertex carrying only a mass needs a physical connection to the model, for
example a structural element or a spring. The exporter reports a disconnected
mass and identifies axial springs whose endpoints coincide.

Select a tree entry to inspect its endpoints, display-unit value and history
assignment. Use **Modify** on the entry to edit it. Geometry-backed
definitions survive remeshing. The Connections drawing toggle displays the
mass and spring locations. The RP tooltip lists masses and springs that use it.

Each feature has its own history assignment. Choose all supported mechanical
steps, a single step, or off. Shared endpoints are printed once per step.
Open the Postprocessor's **Point masses** histories for global nodal motion
or **Springs** for signed extension and derived axial force. Positive axial
spring force denotes tension. The force derives from the exported stiffness
and extension. A positive ground displacement gives a restoring force
opposing that displacement. Physical spring-force histories apply to real
mechanical response steps. Eigenmode displacements have arbitrary normalization.

The example journals are ``examples/calculix/connections/rigid_rp_point_mass.jou``,
``examples/calculix/connections/axial_spring.jou`` and
``examples/calculix/connections/ground_spring.jou``.

Joints
------

Use two distinct cylindrical faces with circular end edges for a shaft and
bore. The coaxial command creates the two endpoint RPs and their rigid
attachments together. The exporter recomputes the common axis and midpoint
of the axial overlap. Both surfaces become rigid attachments. For idealized
joints this choice can materially affect local stresses and bore deformation.

.. code-block:: text

   ccx create joint 1 name "Bearing" type revolute surface 42 to surface 51 coaxial
   ccx create joint 2 name "Support" type spherical rp 5 to ground history all

Alternatively connect two coincident attached RPs using
``rp A to rp B [axis X Y Z]``. A fixed joint locks all relative motion and a
spherical joint allows all relative rotations.

* A revolute joint allows axial rotation. A prismatic joint allows axial travel.
  A cylindrical joint allows both.
* With endpoint B set to Ground, all three support an explicit NLGEOM step.
  The axis is fixed in space and can point in any direction.
* For two moving bodies in a model with an explicit NLGEOM step, use a
  prismatic joint for a slider. A slides along a guide attached to B. Rotating
  B rotates the guide. The two bodies maintain the same rotation.
* A revolute or cylindrical joint between two moving bodies supports large
  rotations when every analysis step is static with NLGEOM active. Use the
  existing RP or coaxial-surface joint command. B carries the guide axis.
  A can twist about it and a cylindrical joint can also slide along it.
  Models combining this formulation with another procedure are rejected at
  export. Use a direct equation solver for the indefinite constraint system.
  Linear models use the small-rotation equations.

The standalone CCX deck in ``examples/calculix/connections/moving_slider.inp``
rotates the guide through 90 degrees and loads the slider along global Y.
Its final travel is 0.1 mm along the rotated guide.
In CCX 2.23 the slider's STRAIGHT constraint can fail when a rotating guide
passes a global coordinate direction. With a translating guide turning
through 120 degrees, the solver reported that it could not change the
dependent degree of freedom. The 90-degree example covers one load path.

History, cleanup and storage
----------------------------

Rigid-body RPs request U and RF at both control nodes by default in static,
dynamic and coupled/uncoupled temperature-displacement steps. Choose history
output in the creation panel or right-click
an RP or joint and choose **History output (U + RF)** to change the step
assignment. Each joint has its own history setting. A joint requests the
endpoint samples it needs without changing either RP's history setting. When
two joints use the same RP, the exporter requests its control-node output once
per step. The REF channels give
translation and nodal external force. The ROT channels give rotation and
generalized nodal moment. RF includes applied loads and reactions.

.. code-block:: text

   ccx rp 5 history step 1
   ccx joint 1 history all
   ccx joint 1 history off
   ccx delete joint 1

Connections are stored in the CalculiX workspace's Connections group. Missing
geometry is handled by CalculiX autocleanup. Removing only the mesh preserves
geometry-backed definitions. A joint created from two faces owns its generated
endpoints. An explicitly referenced RP remains available after joint deletion.
A model saved with rigid bodies under Constraints must be recreated from its
updated journal.

Result conversion reads the RP bindings in the matching input deck and checks
their control nodes and sets. The Postprocessor groups mapped nodal history
tables under **Reference points** and endpoint histories under **Joints**.
A prismatic joint reports signed axial travel, a revolute joint reports signed
axial angle, and a cylindrical joint reports both. Positive travel follows
the joint axis using the displacement of A relative to B. For a moving
nonlinear slider the axis follows B's rotation in that increment. Ground
has zero motion. Endpoint RF is a nodal external force and cannot by itself separate
the reaction of one joint at an endpoint shared by several connections.
Spatial nodeset markers identify each RP and joint at its REF location.
For a moving nonlinear hinge or cylinder the axial angle is the signed relative
twist in radians, wrapped to the interval from minus pi to pi. Generated guide
nodes and constraint springs are excluded from physical result groups and
spring histories when the run mapping is available.
The moving nonlinear joint marker stores its initial axis as ReferenceJointAxis and
its current JointAxis when the guide rotation is available for the frame.
Other directional markers store their exported JointAxis.

For a one-vertex load example see
``examples/calculix/topology/simple_beam.jou``. For an automatically centered
shaft and bore see ``examples/calculix/connections/shaft_bearing.jou``.
