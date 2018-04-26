import inviwopy
from inviwopy.glm import *

v1 = vec3(1,2,3)
v2 = size2_t(4,5)

m1 = mat4(1)
m2 = mat3(0,1,0,-1,0,0,0,0,2)

v3 = m2 * v1

v4 = vec4(1,2,3,4)
w = v4.w
a = v4.a
q = v4.q
z = v4.z
b = v4.b
p = v4.p
y = v4.y
g = v4.g
t = v4.t
x = v4.x
r = v4.r
s = v4.s
