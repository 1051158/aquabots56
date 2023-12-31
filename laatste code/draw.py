import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from matplotlib import animation
import numpy as np
from turtle import *
import turtle as tur

def start_figure():
    #start the figure that is used
    fig = plt.figure()
    ax = fig.add_subplot(projection='3d')
    plt.pause(0.001)
    return fig, ax, plt

def animate_func(num, fig, ax, plt, xyzTest):
    #clear the last graph for new dot.
    plt.cla()

    #set limits for the graph
    ax.set_ylim3d(ymin=0, ymax=10)
    ax.set_xlim3d(xmin=0, xmax=10)
    ax.set_zlim3d(zmin= 0, zmax=1)

    #make normal floats out of the incoming array
    x = num[0]
    y = num[1]
    if xyzTest:
        z = num[2]
        ax.set_zlabel('z: ' + str(z))

    #give names to the axis and graph
    ax.set_title('tag_position')
    ax.set_xlabel('x: ' + str(x))
    ax.set_ylabel('y: ' + str(y))


    #plot the graph
    if xyzTest:
        ax.scatter(x, y, z, 'o', color='red')
    else:
        ax.scatter(x, y, 0.87, 'o', color='red')


    plt.pause(0.00001)
    return fig, ax, plt



