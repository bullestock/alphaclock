from build123d import *
from ocp_vscode import *
from defs import *

bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    Box(ow, ow, bearing_th, align=bottom)
    Cylinder(bearing_od/2 + b_crush/2, bearing_th, align=bottom,
             mode=Mode.SUBTRACT)
    # crush ribs
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with PolarLocations(radius=bearing_od/2 + b_crush/2, count=10):
            Circle(b_crush)
    extrude(amount=-bearing_th)
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with PolarLocations(radius=screw_radius, count=4, start_angle=45):
            Circle(3.2/2)
    extrude(amount=-bearing_th, mode=Mode.SUBTRACT)
    
show(p)

export_step(p.part, 'mount.step')
