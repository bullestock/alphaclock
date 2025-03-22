# %%
from build123d import *
from ocp_vscode import *

H = 12
depth = 16

pts = [
    (15, 0),
    (28, 0),
    (26, H),
    (0, H),
    (0, H - 2),
    (15, 6),
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
export_step(o.part, "stepperholder-secs-2.step")
