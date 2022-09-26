# This is a Python script to parse the example messages from a file.
from re import X
from mpl_toolkits import mplot3d
from collections import namedtuple
import numpy as np
from matplotlib import pyplot as plt
import time
import serial
import struct
import traceback
import math

MSG_HEADER_SIZE = 2
x_distances = []
y_distances = []
z_distances = []

f = serial.Serial(port="COM4", baudrate=9600, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)
read_byte = f.read()

struct_element = 0
sentinel = 2
end = 0

while sentinel < 5:

    # find way to check for sentinel
    while sentinel > 1:
        read_byte = f.read(1)

        decimal = int.from_bytes(read_byte, "big", signed = True)

        # waits for sentinel
        if(decimal == -85):
            read_byte = f.read(1)

            sentinel = 0
            struct_element = struct_element+1

    # Close current figure if there is one open
    plt.close('all')

    read_byte = f.read(MSG_HEADER_SIZE)

    decimal = int.from_bytes(read_byte, "big", signed = True)

    # If value is 0 store the 0 to see if there are 4 in a row
    if(decimal == 0):
        end = end + 1
    else:
        end = 0

    if(struct_element == 1):
        y_distances.append(decimal)
    elif(struct_element == 2):
        x_distances.append(-decimal)
    elif(struct_element == 3):
        z_distances.append(decimal)
    elif(struct_element == 4):
        struct_element = -1

    # leave loop if no longer receiving samples
    if(end == 5):
        end = 0
        sentinel = 2
        sum = 0

        # Concatenate arrays
        x = np.array(x_distances)
        y = np.array(y_distances)
        z = np.array(z_distances)

        # Filtering data
        indices = []

        for i in range(0, len(x)-1):
            if((y[i]<300) or (y[i]>900)):
                indices.append(i)

        x = np.delete(x, indices)
        y = np.delete(y, indices)
        z = np.delete(z, indices)

        noise = []

        for i in range (1,len(x)-2):
            y_prev = y[i-1]
            y_current = y[i]
            y_next = y[i+1]

            if((y_prev + 10 < y_current and y_next + 10 < y_current) or (y_prev - 10 > y_current and y_next - 10 > y_current)):
                noise.append(i)

        x = np.delete(x, noise)
        y = np.delete(y, noise)
        z = np.delete(z, noise)

        #xy = np.vstack((x, y))
        xy = np.array([x, y])
    
        xy_sorted = xy[:, xy[0,:].argsort()]

        x = xy_sorted[0,:]

        # Shift x values by systematic error
        for i in range(0, len(x) - 1):
            x[i] = x[i] + 100

        y = xy_sorted[1,:]
        #z = xy_sorted[2,:]

        #x_flat = np.array(x)
        #y_flat = np.array(y)
        
        #indicesflat = []

        #for i in range(0, len(z)-1):
        #    if((z!=0)):
        #        indices.append(i)

        #for i in range(0, len(x) - 1):
            #print(x[i])

        # Find the area under the curve of the outline of the stock (35cm away from front of stock, 32 degree angle scan)
        width = 440
        depth = 500

        total_area = width*depth

        for i in range(0, len(x)-1): #xdist - 1 to avoid indexing out of range. 

            trap_area = ((y[i]-350)+(y[i+1]-350))*abs((x[i+1]-x[i]))*0.5

            # Check not scanning beyond stock area
            if(x[i] > 220 or x[i] < -220):
                trap_area = 0

            sum = sum + trap_area     

        percentage = ((total_area-sum)/total_area)*100
        print(percentage)

        # Plot the figure of depths and widths (birds eye view of shelf)
        plt.plot(x, y, 'o')
        #ax = plt.axes(projection = '3d')
        #ax.scatter3D(x, y, z, c = y, cmap = 'Greens')

        plt.xlim([-220, 220])
        plt.ylim([0, 900])
        plt.show()

        # Clear arrays 
        x_distances.clear()
        y_distances.clear()
        z_distances.clear()

        # clear numpy arrays
        del x
        del y
        del z
    
    struct_element = struct_element + 1