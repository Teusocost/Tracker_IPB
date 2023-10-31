def ray_casting(point, polygon):
    intersections = 0
    x, y = point

    for k in range(len(polygon)-1):
        x1, y1 = polygon[k]
        x2, y2 = polygon[k+1]

        if (y < y1) != (y < y2) and\
        x < (x2-x1)*(y-y1)/(y2-y1)+x1:
            intersections += 1

    return intersections % 2 == 1

def point_on_side(point, polygon): # when the point lies on a side of the polygon
    is_on_a_side = False
    x, y = point

    for k in range(len(polygon)-1):
        x1, y1 = polygon[k]
        x2, y2 = polygon[k+1]

        if (y < y1) != (y < y2) and\
        (x < x1) != (x < x2) and\
        (x2-x1)*(y-y1)==(x-x1)*(y2-y1):
            is_on_a_side = True
            break

    return is_on_a_side



from random import uniform
from numpy import array
from matplotlib import pyplot as plt

X = []
Y = []
P = [
    [1.31,3.53], [0.32,1.49],
    [0.32,0.18], [3.22,0.18],
    [4.12,2.03], [1.2,1.72],
    [2.19,3.76], [1.5,4.32],
    [0.35,3.53], [1.31,3.53]
]
colors = []

for i in range(2000):
    x = uniform(-1, 5)
    y = uniform(-1, 5)

    X.append(x)
    Y.append(y)

    if ray_casting([x,y], P):
        colors.append('green')
    else:
        colors.append('red')



x1 = P[5][0]
y1 = P[5][1]
x2 = P[6][0]
y2 = P[6][1]

x = (x1+x2)/2
y = ((y2-y1)/(x2-x1))*(x-x1) + y1

_point_on_side = [x,y]

plt.plot(array(P)[:,0], array(P)[:,1], color='gray', linewidth = 1)

plt.scatter(X, Y, c=colors, s=2)

plt.scatter([x], [y], c='blue', s=8)

plt.show()