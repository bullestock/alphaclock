from build123d import *
from ocp_vscode import *

od = 272
w = 35
th = 2

bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    Cylinder(od/2, w, align=bottom)
    Cylinder(od/2 - th, w, align=bottom, mode=Mode.SUBTRACT)
    split(bisect_by=Plane.XZ.offset(0 / 2))
    split(bisect_by=Plane.YZ.offset(0 / 2))
    with BuildSketch(Plane.XY):
        with Locations((od/2 - 1.25*th, 2)):
            Rectangle(1.5, 10)
    extrude(amount=w)

show(p)

export_step(p.part, 'ring.step')
