from build123d import *
from ocp_vscode import *
from defs import *

worm_l = 7
spacer_l = 5
th = 2.5
stud_h = 5
stud_d = 5
stud_w = 5
w = 2*stud_w + worm_l + 2*spacer_l
d = 16
screwhole_y = 4.5
stud_y = -7.1
axle_y = -4.
axle_z = 3


bottom = (Align.CENTER, Align.CENTER, Align.MIN)

with BuildPart() as p:
    # basic shape
    with BuildSketch():
        RectangleRounded(w, d, 1)
    extrude(amount=th)
    # screw holes
    with BuildSketch():
        with Locations(p.faces().sort_by(Axis.Z)[-1]) as sk:
            with Locations((-5, screwhole_y), (5, screwhole_y)):
                RectangleRounded(3.2, 5, 1.5)
    extrude(amount=th, mode=Mode.SUBTRACT)
    # worm cutout
    with BuildSketch():
        with Locations((0, -5, 0)):
            RectangleRounded(worm_l + 2*spacer_l, 10, 1)
    extrude(amount=th, mode=Mode.SUBTRACT)
    # studs
    with BuildSketch(p.faces().sort_by(Axis.Z)[-1]):
        with Locations((-(worm_l/2 + stud_w/2 + spacer_l), stud_y, 0), 
                       ((worm_l/2 + stud_w/2 + spacer_l), stud_y, 0)):
            RectangleRounded(stud_w, stud_d, 1)
    extrude(amount=stud_h)
    # axle hole
    with BuildSketch(p.faces().sort_by(Axis.X)[-1]):
        with Locations((axle_y, axle_z)):
            Circle(2.1/2)
    extrude(amount=-w, mode=Mode.SUBTRACT)
    #e = p.edges().sort_by(SortBy.LENGTH)
    #fillet([e[9], e[14], e[-1]], radius=0.15)
    

show(p)

export_step(p.part, 'wormholder.step')
