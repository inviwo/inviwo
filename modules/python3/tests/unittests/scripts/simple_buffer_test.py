import numpy as np

a = intBuffer.data[1] + 0 # plus zero to make a copy or a&b is changed in the for loop
b = intBuffer.data[3] + 0 

for i in range(0,intBuffer.data.shape[0]):
    intBuffer.data[i] = i*i    
