from PIL import Image
import numpy as np


#fake values for updating img
stock = 69
position = 5

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
    masks = [_1L1, _1R1, _1L2, _1R2, _2L1, _2R1, _2L2, _2R2]
    coordinates = [(490,1750),(1320,1750),(490,1100),(1320,1100),(490,840),(1320,840),(490,200),(1320,200)]

    stock_coords = coordinates[position]

    left_num = int(np.floor(stock/10))
    right_num = stock - 10*left_num

    left_num_im = nums[left_num]
    right_num_im = nums[right_num]

    if (stock == 100):
        stock_percentage = Image.new('RGBA',(180,46), color=(255,255,255,0))
        stock_percentage.paste(left_num_im,(0,0),left_num_im)
        stock_percentage.paste(right_num_im,(70,0),right_num_im)
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

    if (stock < 25):
        colour = red
    elif (stock < 50):
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

background = stocktake(69,0,background)
background = stocktake(0,5,background)
background = stocktake(100,2,background)
background = stocktake(15,3,background)
background = stocktake(42,4,background)
background = stocktake(0,5,background)
background = stocktake(99,6,background)
background = stocktake(35,7,background)
