#!/usr/bin/env python3

import sys, os.path

if len(sys.argv) != 2:
  print("Usage: %s <data.csv>" % sys.argv[0])
  exit(1)
filename = sys.argv[1]
if not os.path.isfile(filename):
  print("Error: %s is not a valid file" % filename)
  exit(1)

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

delta = 100
smooth = 1

data = pd.read_csv(filename).set_index("Nb")
data["Total"] = data["Input"] + data["Output"]
data.plot()

plt.show()
