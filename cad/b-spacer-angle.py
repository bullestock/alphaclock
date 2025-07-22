from build123d import *
from ocp_vscode import *

th = 5
cd = 25
height = 11.5
width = 40
z_offset = 0.5 - 4.75 + th/2

bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    Box(width, width, th, align=bottom)
    Cylinder(cd/2 + 2, th, mode=Mode.SUBTRACT, align=bottom)
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]) as sk:
        with PolarLocations(radius=22, count=4, start_angle=45):
            Circle(3.2/2)
    extrude(amount=-th, mode=Mode.SUBTRACT)
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1].offset(-th)):
        with Locations((0, width/2 + th/2)):
            Rectangle(width, th)
    extrude(amount=height)
    with BuildSketch(p.faces().sort_by(Axis.Y)[-1]) as sk:
        with Locations([(12.5, z_offset), (-12.5, z_offset)]):
            Circle(radius=3/2)
    extrude(amount=-20, mode=Mode.SUBTRACT)
    
show(p)

export_step(p.part, 'b-spacer-angle.step')
