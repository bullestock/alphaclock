# %%
from build123d import *
from ocp_vscode import *

length, height, width = 10, 10, 30
bend_radius = 1
thickness = 2
hole_cc = 23.1

with BuildPart() as bracket:
    with BuildSketch() as sketch:
        with BuildLine() as profile:
            FilletPolyline(
                (0, 0), (length, 0), (length, height), radius=bend_radius
            )
            offset(amount=thickness, side=Side.LEFT)
        make_face()
    extrude(amount=width)
    fillet(bracket.edges().filter_by(Axis.Z), radius=0.1)
    fillet(bracket.edges().filter_by(Axis.Y), radius=0.1)
    # screw holes
    with BuildSketch(bracket.faces().sort_by(Axis.Y)[-1]) as h_sk:
        with Locations([(length/2, -hole_cc/2), (length/2, hole_cc/2)]):
            Circle(radius=3.2/2)
    extrude(amount=-50, mode=Mode.SUBTRACT)

show(bracket)
export_step(bracket.part, "stepperholder-fixed.step")
