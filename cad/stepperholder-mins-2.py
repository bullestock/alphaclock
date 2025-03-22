# %%
from build123d import *
from ocp_vscode import *

H = 10
depth = 16

pts = [
    (7, 0),
    (20, 0),
    (18, H),
    (0, H),
    (0, 8),
    (7, 6),
]

with BuildPart() as o:
    with BuildSketch(Plane.YZ) as o_sk:
        with BuildLine() as o_ln:
            Polyline(pts, close=True)
        make_face()
    extrude(amount=depth)
    fillet(o.edges().filter_by(Axis.Z), radius=1)
    fillet(o.edges().sort_by(Axis.Z)[-1], radius=1)

show(o)    
export_step(o.part, "stepperholder-mins-2.step")
