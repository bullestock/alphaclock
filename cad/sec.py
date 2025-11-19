from build123d import *
from ocp_vscode import *
from defs import *

i_dia = 2.1
rr = 0.5

p_dia = 65
slot_dia = 2
p_th = 4.5

int_th = 1

bearing_clearance = 0.5

bearing_z = p_th

bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    # gear mount with screw holes
    with BuildSketch():
        Circle(gear_inner_dia/2)
    extrude(amount=gear_mount_th)
    with Locations(p.faces().sort_by(Axis.Z)[-1]):
        with PolarLocations(radius=gear_hole_cc/2, count=4):
            CounterSinkHole(gear_hole_dia/2, gear_hole_dia)
    # spacer
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]):
        Circle(bearing_id/2 + 1.5)
    extrude(amount=bearing_clearance)
    # part that fits inside bearing
    with Locations([(0, 0, bearing_z)]):
        Cylinder(bearing_id/2 - b_crush/2, bearing_th)
    # bearing crush ribs
    with BuildSketch(p.faces().sort_by(Axis.Z)[0].offset(-gear_mount_th - rr)) as sk:
        with PolarLocations(radius=bearing_id/2 - b_crush/2 - 0.1, count=10):
            Circle(b_crush)
    extrude(amount=-(bearing_th - 2*rr))
    # through hole
    Cylinder(i_dia/2, 20, align=bottom,
             mode=Mode.SUBTRACT)

show(p)

export_step(p.part, 'sec.step')
