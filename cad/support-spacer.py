import cadquery as cq

# overall
dia = 150
# raised part
dia2 = 25
# hole
dia3 = 17.3
w = 20
d = 8
th = 2
h = 17
insert_r = 4.2/2

# base
part1 = (cq.Workplane("XY")
         .circle(dia/2)
         .extrude(th)
         .circle(dia2/2)
         .extrude(th)
         )

# uprights
part2 = (cq.Workplane("XY")
         .rect(w, dia + 2 * d)
         .extrude(th)
         .faces(">Z")
         .rarray(1, dia + d, 1, 2)
         .rect(w, d)
         .extrude(h)
         .faces(">Z")
         .workplane()
         .rarray(w/2, dia + d, 2, 2)
         .circle(insert_r)
         .cutBlind(-5)
         .edges("|Z")
         .fillet(2)
         )

res = part1.union(part2)

# center hole
res = (res
         .faces(">Z")
         .workplane()
         .circle(dia3/2)
         .cutThruAll()
)
show_object(res)
