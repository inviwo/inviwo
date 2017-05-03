import numpy as np

a = intBuffer.data[1] + 0 # plus zero to make a copy
b = intBuffer.data[3] + 0 # or a&b is changed in the for loop

for i in range(0,intBuffer.data.shape[0]):
    intBuffer.data[i] = i*i    



c = np.array([1,2,3]);
d = np.array([1,2,3.0]);
e = np.array([[1,2],[3,4.0]]);

def v(x):
    print(x.shape);
    print(x.dtype);

v(c)
v(d)
v(e)