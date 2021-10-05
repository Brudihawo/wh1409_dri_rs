import os
import sys

import numpy as np
import pandas as pd
from matplotlib import pyplot as plt

fname = sys.argv[1]

datapoints = pd.read_table(fname, sep="\s+",
                           skiprows=1,
                           names=["status", "vert", "horiz", "pressure"])

fig, ax = plt.subplots(2, 2)

ax[0][0].plot(datapoints["horiz"].rolling(40).mean())
ax[0][0].set_title("Horizontal Signal")

ax[0][1].plot(datapoints["vert"].rolling(40).mean())
ax[0][1].set_title("Vertical Signal")

ax[1][0].plot(datapoints["pressure"].rolling(40).mean())
ax[1][0].set_title("Pressure")


ax[1][1].scatter(datapoints["horiz"], datapoints["vert"],
                 c=np.linspace(0, 1, len(datapoints)))
ax[1][1].set_title("Horizontal and Vertical Component")
ax[1][1].set_xlabel("Horizontal Component")
ax[1][1].set_ylabel("Vertical Component")

plt.tight_layout()
fig.savefig(f"img/{os.path.basename(fname.replace('.tsv', ''))}", dpi=600)

plt.show()
plt.show()
