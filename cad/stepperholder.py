# %%
from build123d import *
from ocp_vscode import *

depth = 10
mot_d = 20
offset = 4
width = mot_d + 5
factor = 0.6

with BuildPart() as o:
    # basic shape
    Box(width, depth, offset + mot_d * factor, align=Align.MIN)
    # fillet
    fillet(o.edges().filter_by(Axis.Z), radius=1)
    fillet(o.edges().sort_by(Axis.Z)[-1], radius=1)
    # motor hole
    with BuildSketch(o.faces().sort_by(Axis.Y)[-1]) as h:
        with Locations([ (offset, 0, 0) ]):
            Circle(radius=mot_d/2)
    extrude(amount=-width, mode=Mode.SUBTRACT)

show(o)    
export_step(o.part, "stepperholder.step")
