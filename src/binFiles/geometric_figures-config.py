gf.echo("Warning: gf module interface (used in python scripts) will be changed in future version.");

# Rotation:
 #2D:
gf.rmap("q", 1, 2)
gf.map("Q", "rot 1 2 15")
gf.rmap("w", 2, 1)
gf.map("W", "rot 2 1 15")
 #3D:
gf.rmap("a", 1, 3)
gf.map("A", "rot 1 3 15")
gf.rmap("s", 3, 1)
gf.map("S", "rot 3 1 15")
gf.rmap("z", 2, 3)
gf.map("Z", "rot 2 3 15")
gf.rmap("x", 3, 2)
gf.map("X", "rot 3 2 15")
 #4D:
gf.rmap("e", 3, 4)
gf.map("E", "rot 3 4 15")
gf.rmap("r", 4, 3)
gf.map("R", "rot 4 3 15")
gf.rmap("d", 1, 4)
gf.map("D", "rot 1 4 15")
gf.rmap("f", 4, 1)
gf.map("F", "rot 4 1 15")
gf.rmap("c", 2, 4)
gf.map("C", "rot 2 4 15")
gf.rmap("v", 4, 2)
gf.map("V", "rot 4 2 15")

# Figures:
gf.map("<Esc>", "close")
 #0D:
gf.map("~", "o %/data/0d.dat")
 #1D:
gf.map("`", "o %/data/1d-2.dat")
 #2D:
gf.map("1", "o %/data/2d-3.dat")
 #3D:
gf.map("2", "{o %/data/3d-4.dat;    rot 1 3 30; rot 2 3 20}")
gf.map("3", "{o %/data/3d-6.dat;    rot 1 3 30; rot 2 3 20}")
gf.map("4", "{o %/data/3d-8.dat;    rot 1 3 30; rot 2 3 20}")
gf.map("5", "{o %/data/3d-12.dat;   rot 1 3 30; rot 2 3 20}")
gf.map("6", "{o %/data/3d-20.dat;   rot 1 3 30; rot 2 3 20}")
 #4D:
gf.map("7", "{o %/data/4d-5.dat;    rot 1 3 30; rot 2 3 20}")
gf.map("8", "{o %/data/4d-8.dat;    rot 1 3 30; rot 2 3 20}")
gf.map("9", "{o %/data/4d-16.dat;   rot 1 3 30; rot 2 3 20}")
gf.map("0", "{o %/data/4d-24.dat;   rot 1 3 30; rot 2 3 20}")
gf.map("-", "{o %/data/4d-120.dat;  rot 1 3 30; rot 2 3 20}")
gf.map("=", "{o %/data/4d-600.dat;  rot 1 3 30; rot 2 3 20}")
 #5D:
gf.map("+", "{so %/data/5d-10.txt;   rot 1 3 30; rot 2 3 20}") # changes settings

# Modification:
gf.map("p", "vertex previous")
gf.map("n", "vertex next")
gf.map("o", "vertex deselect")
gf.map("P", "vertex remove")
gf.map("N", "vertex add")
gf.map("h", "vertex move -0.2")
gf.map("l", "vertex move 0.2")
gf.map("j", "vertex move 0 -0.2")
gf.map("k", "vertex move 0 0.2")
gf.map("u", "vertex move 0 0 -0.2")
gf.map("i", "vertex move 0 0 0.2")
gf.map("m", "vertex move 0 0 0 -0.2")
gf.map(",", "vertex move 0 0 0 0.2")

# Four small letters remain:
gf.map("b", "reset boundary")
gf.map("g", "reset rotation")
gf.map("t", "set noconvexhull")
gf.map("y", "set convexhull")

# Predefined color modes
gf.map("<f1>", "reset colors")
gf.map("<f2>", '''{
set spacecolor=gray;
set spacecolor-3=blue;
set spacecolor+3=yellow;
set spacecolor-4=transparent;
set spacecolor+4=transparent}''') # 3D axis colored
gf.map("<f3>", '''{
set spacecolor=gray;
set spacecolor+3=transparent;
set spacecolor-3=transparent;
set spacecolor-4=red;
set spacecolor+4=green}''') # 4D axis colored
gf.map("<f4>", '''{
set spacecolor=gray;
set spacecolor-3=blue;
set spacecolor+3=yellow;
set spacecolor-4=red;
set spacecolor+4=green}''') # 3D and 4D axes colored

gf.map("<f5>", "set facecolor=0.2 gray")
gf.map("<f6>", "set facecolor=0.03 white")
gf.map("<f7>", "set facecolor=transparent")

# Predefined edges and vertices sizes
gf.map("<f8>", '''{
set edgesize=20;
set selvertsize=20}''')
gf.map("<f9>", '''{
set edgesize=10;
set selvertsize=10}''')
gf.map("<f10>", '''{
set edgesize=5;
set selvertsize=5}''')
gf.map("<f11>", '''{
set edgesize=2;
set selvertsize=2}''')
gf.map("<f12>", '''{
set edgesize=1;
set selvertsize=1}''')