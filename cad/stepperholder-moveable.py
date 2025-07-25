# %%
from build123d import *
from ocp_vscode import *
from defs import *

depth = 10
mot_d = 19.8
offset = 3
width = 25
hole_cc = 23 # motor_bracket_cc
flange_extend = 3
flange_width = width + 4*flange_extend
flange_th = 5
factor = 0.8
INSERT_R = 4/2
z_offset = 0 #1.5

print(width + flange_extend)

with BuildPart() as o:
    with BuildSketch():
        # flange
        Rectangle(flange_width, depth)
    extrude(amount=flange_th)
    with BuildSketch(Plane.XY.offset(flange_th)):
        # basic shape
        Rectangle(width, depth)
    extrude(amount=offset + mot_d * factor - flange_th)
    # fillet
    fillet(o.edges().filter_by(Axis.Z), radius=1)
    fillet(o.edges().sort_by(Axis.Z)[-1], radius=1)
    # motor hole
    with BuildSketch(o.faces().sort_by(Axis.Y)[0]) as h:
        with Locations([ (0, offset, 0) ]):
            Circle(radius=mot_d/2)
    extrude(amount=-width, mode=Mode.SUBTRACT)
    # screw holes
    with BuildSketch(o.faces().sort_by(Axis.Z)[0]):
        with Locations([(-hole_cc/2, z_offset, 0), (hole_cc/2, z_offset, 0)]):
            Circle(radius=3.4/2)
    extrude(amount=-6, mode=Mode.SUBTRACT)
    

show(o)    
export_step(o.part, f"stepperholder-moveable-{z_offset}.step")
