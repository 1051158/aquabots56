import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from matplotlib import animation
import numpy as np

def start_figure():
    plt.ion()
    # Adding Figure Labels
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    ax.set_xlim3d([-2, 2])
    ax.set_ylim3d([-2, 2])
    ax.set_zlim3d([0, 2])
    ax.set_title('tag_position')
    ax.set_xlabel('x')
    ax.set_ylabel('y')
    ax.set_zlabel('z')
    return fig

def animate_func(num, fig):
    print(num)
    x = num[0]
    y = num[1]
    z = num[2]

    plt.scatter(x, y, z, 'r', marker='o')
    return fig



