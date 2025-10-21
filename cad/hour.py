from build123d import *
from ocp_vscode import *
from defs import *

# Moving part for hours

bearing_id = 20
bearing_th = 4
stem_h = 14.5
o_dia1 = 5.9
o_dia2 = 7
mount_h = 1.5
i_dia = 4.2
b_crush = 0.45
h_crush = 0.2
rr = 0.5

gear_inner_dia = 40
gear_hole_cc = 30
gear_hole_dia = 2.2
gear_mount_th = 2

bearing_clearance = 1

bearing_z = gear_mount_th + bearing_clearance

bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    # gear mount with screw holes
    Cylinder(gear_inner_dia/2, gear_mount_th)
    with Locations(p.faces().sort_by(Axis.Z)[-1]):
        with PolarLocations(radius=gear_hole_cc/2, count=4):
            CounterSinkHole(gear_hole_dia/2, gear_hole_dia)
    # spacer
    with Locations([(0, 0, bearing_z/2)]):
        Cylinder(bearing_id/2 + 1.5, bearing_clearance)
    # part that fits inside bearing
    with Locations([(0, 0, bearing_z)]):
        Cylinder(bearing_id/2 - b_crush/2, bearing_th)
    # bearing crush ribs
    with BuildSketch(p.faces().sort_by(Axis.Z)[0].offset(-gear_mount_th - rr)) as sk:
        with PolarLocations(radius=bearing_id/2 - b_crush/2 - 0.1, count=10):
            Circle(b_crush)
    extrude(amount=-(bearing_th - 2*rr))
    # reinforced part
    Cylinder(o_dia2/2 - h_crush/2, stem_h - mount_h, align=bottom)
    # mount for hand
    Cylinder(o_dia1/2 - h_crush/2, stem_h, align=bottom)
    e = p.edges().sort_by(Axis.Z)
    fillet([e[0], e[-1]], radius=rr)
    # hand crush ribs
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1].offset(-rr)) as sk:
        with PolarLocations(radius=o_dia1/2 - h_crush/2, count=10):
            Circle(h_crush)
    extrude(amount=-2)
    # through hole
    Cylinder(i_dia/2, 50,
             mode=Mode.SUBTRACT)
    
show(p)

export_step(p.part, 'hour.step')
