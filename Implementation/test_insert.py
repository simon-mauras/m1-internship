#!/usr/bin/env python3


import subprocess as sub
import numpy as np
import pandas as pd
from matplotlib import pyplot as plt
from matplotlib import animation

fig = plt.figure()
ax = plt.axes()

def init():
  pass

def animate(B):
  sub.call(["./test_insert", str(B), "bst"])
  data = pd.read_csv("insert.csv").set_index("Nb") + 1
  ax.clear()
  data.plot(ax=ax)
  ax.set_xlabel("N° of insertions")
  ax.set_ylabel("N° of block I/O")
  ax.set_title("BlockSize = %d" % B)
  ax.legend(loc=2)

anim = animation.FuncAnimation(fig, animate, init_func=lambda:None,
                               frames=range(10, 501, 10),
                               repeat=False)
anim.save("insert.mp4")
plt.show()
