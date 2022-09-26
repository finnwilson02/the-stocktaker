# This is a Python script to parse the example messages from a file.
from re import X
from xml.dom.minidom import ProcessingInstruction
from mpl_toolkits import mplot3d
from collections import namedtuple
import numpy as np
from matplotlib import pyplot as plt
import time
import serial
import struct
import traceback
import math
from PIL import Image

MSG_HEADER_SIZE = 2
x_distances = []
y_distances = []
z_distances = []
background = Image.open(r"base.png")

def stocktake(stock,position,background):

        #import background and different shelf masks
    _1L1 = Image.open(r"1L1.png")
    _1L2 = Image.open(r"1L2.png")
    _2L1 = Image.open(r"2L1.png")
    _2L2 = Image.open(r"2L2.png")
    _1R1 = Image.open(r"1R1.png")
    _1R2 = Image.open(r"1R2.png")
    _2R1 = Image.open(r"2R1.png")
    _2R2 = Image.open(r"2R2.png")

    #import numbers 
    _0 = Image.open(r"0.png")
    _1 = Image.open(r"1.png")
    _2 = Image.open(r"2.png")
    _3 = Image.open(r"3.png")
    _4 = Image.open(r"4.png")
    _5 = Image.open(r"5.png")
    _6 = Image.open(r"6.png")
    _7 = Image.open(r"7.png")
    _8 = Image.open(r"8.png")
    _9 = Image.open(r"9.png")
    _10 = Image.open(r"10.png")
    _percent = Image.open(r"%.png")

    nums = [_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_percent] #make numbers an array for ez indexing
    masks = [_1L1, _1R1, _1R2, _1L2, _2L1, _2R1, _2R2, _2L2]
    coordinates = [(490,1750),(1320,1750),(1320,1100),(490,1100),(490,840),(1320,840),(1320,200)(490,200),]

    stock_coords = coordinates[position]

    left_num = int(np.floor(stock/10))
    right_num = stock - 10*left_num

    left_num_im = nums[left_num]
    right_num_im = nums[right_num]

    if (stock >= 100):
        stock_percentage = Image.new('RGBA',(180,46), color=(255,255,255,0))
        stock_percentage.paste(_10,(0,0),_10)
        stock_percentage.paste(_0,(70,0),_0)
        stock_percentage.paste(_percent,(110,0),_percent)
    else:
        stock_percentage = Image.new('RGBA',(140,46), color=(255,255,255,0))
        stock_percentage.paste(left_num_im,(0,0),left_num_im)
        stock_percentage.paste(right_num_im,(40,0),right_num_im)
        stock_percentage.paste(_percent,(80,0),_percent)

    red = (255,0,0)
    orange = (255,165,0)
    yellow = (255,224,100)
    green = (0,255,0)

    if (stock == 0):
        colour = red
    elif (stock < 25):
        colour = orange
    elif(stock < 50):
        colour = yellow
    else:
        colour = green


    mask = masks[position]
    img = mask.convert('RGBA')
    background = background.convert('RGBA')
    d = img.getdata()
    new_image = []

    for item in d:
    
        # change all white (also shades of whites)
        # pixels to yellow
        if item[0] == 255 and item[1] == 255 and item[2] == 255 and item[3] != 0:
            new_image.append(colour)
        else:
            new_image.append(item)

    img2 = Image.new("RGBA",img.size)
    img2.putdata(new_image)
    img2.paste(stock_percentage,stock_coords,stock_percentage)
    background.paste(img2,(108,-1),img2)
    background.show()
    return background

f = serial.Serial(port="COM4", baudrate=9600, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)
read_byte = f.read()

struct_element = 0
sentinel = 2
end = 0
position = -1

while sentinel < 5:

    # find way to check for sentinel
    while sentinel > 1:
        read_byte = f.read(1)

        decimal = int.from_bytes(read_byte, "big", signed = True)

        # waits for sentinel
        if(decimal == -85):
            read_byte = f.read(1)

            sentinel = 0
            struct_element = 1

    # Close current figure if there is one open
    #plt.close('all')

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
    # Enter code for area
    elif(struct_element == 4):
        if(position == -1):
            position = decimal
        struct_element = -1
    #elif(struct_element == 5):
        #struct_element = -1

    # leave loop if no longer receiving samples
    if(end == 5):

        sum = 0

        # Concatenate arrays
        x = np.array(x_distances)
        y = np.array(y_distances)
        z = np.array(z_distances)

        # Sort data
        #xy = np.vstack((x, y))
        xy = np.array([x, y, z])
    
        xy_sorted = xy[:, xy[0].argsort()]
        #xy_sorted = xy[:, xy[0,:].argsort()]

        x = xy_sorted[0,:]
        y = xy_sorted[1,:]
        z = xy_sorted[2,:]

        # Shift x values by systematic error
        for i in range(0, len(x) - 1):
            x[i] = x[i] + 100

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

        # Find the area under the curve of the outline of the stock (35cm away from front of stock, 32 degree angle scan)
        width = 440
        depth = 500

        total_area = width*depth
        indices.clear()

        filter_arr = z>0
        x_bottom = x[filter_arr]
        y_bottom = y[filter_arr]

        for i in range(0, len(x_bottom)-1): #xdist - 1 to avoid indexing out of range. 

            trap_area = ((y_bottom[i]-350)+(y_bottom[i+1]-350))*abs((x_bottom[i+1]-x_bottom[i]))*0.5

            # Check not scanning beyond stock area
            if(x_bottom[i] > 220 or x_bottom[i] < -220):
                indices.append(i)
                trap_area = 0
            #if(z[i] != 0):
                #print(z[i])
                #trap_area = 0

            sum = sum + trap_area     

        percentage = ((total_area-sum)/total_area)*100

        x = np.delete(x, indices)
        y = np.delete(y, indices)
        z = np.delete(z, indices)

        if(percentage<0):
            percentage = 0
        percentage = np.floor(percentage/5)*5
        background = stocktake(int(percentage),position,background)

        # Plot the figure of depths and widths (birds eye view of shelf)
        # plt.plot(x, y, 'o')
        ax = plt.axes(projection = '3d')
        ax.scatter3D(x, y, z, c = y, cmap = 'inferno')

        plt.xlim([-220, 220])
        plt.ylim([300, 1100])
        plt.show()

        # Clear arrays 
        x_distances.clear()
        y_distances.clear()
        z_distances.clear()
        del x_bottom
        del y_bottom

        # clear numpy arrays
        del x
        del y
        del z

        end = 0
        sentinel = 2
        position = -1

    struct_element = struct_element + 1

