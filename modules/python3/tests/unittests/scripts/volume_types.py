import numpy as np

def createArray(a , shape , type):
    arr = np.array(a,dtype=type)
    arr.shape = shape
    return arr
    

arrs = []
a1d = [1,2,3,4,5,6,7,8]
a2d = [[1,2], [3,4], [5,6] , [7,8] , [9, 10] , [11,12] , [13,14] , [15,16]]
a3d = [[1,2,3], [4,5,6], [7,8,9] , [10,11,12],[13,14,15], [16,17,18], [19,20,21] , [22,23,24]]
a4d = [[1,2,3,4], [5,6,7,8], [9,10,11,12], [13,14,15,16] , [17,18,19,20], [21,22,23,24], [25,26,27,28], [29,30,31,32]]

types = [np.float16,np.float32,np.float64,np.int8,np.int16, np.int32,np.int64,np.uint8,np.uint16,np.uint32,np.uint64]
for t in types:
    a = createArray(a1d,(2,2,2),t)
    b = createArray(a2d,(2,2,2,2),t)
    c = createArray(a3d,(2,2,2,3),t)
    d = createArray(a4d,(2,2,2,4),t)

    arrs.append(a)
    arrs.append(b)
    arrs.append(c)
    arrs.append(d)












