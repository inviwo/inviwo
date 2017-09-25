import numpy as np
import math

data = vol.data
size = data.shape 

#x and y are in pixel coordinates
def toLoc( x,y,z ):
    a = x/(size[0]-1)
    b = y/(size[1]-1)
    c = z/(size[2]-1)

    a = a * 2 - 1;
    b = b * 2 - 1;
    c = c * 2 - 1;

    return [a,b,c]

def len2(x,y,r):
    return math.sqrt(x*x+y*y) * r

def len3(x,y,z,r):
    return math.sqrt(x*x+y*y+z*z) * r

objs = [
    (lambda x,y,z : len2(x,y,1)) , 
    (lambda x,y,z : len2(y,z,0.8)) , 
    (lambda x,y,z : len2(z,x,0.6)) ,  
    (lambda x,y,z : len3(x,y,z,0.2)) 
]

if True: 
    for k in range(0,size[2]):
        for j in range(0,size[1]):
            for i in range(0,size[0]):
                (x,y,z) = toLoc(i,j,k);

                d = 100;
                for o in objs:
                    d = min(d , o(x,y,z))
                data[i,j,k] = 1-d;
