import math

YRANGE = [-385, -143, -27, 27, 143, 385]
PARAM = [378, 91, 58]
NUMPOINTS = 360 # actually NUMPOINTS + 3

point_info = []
layers = []

def part1(y: float):
    global point_info, layers

    r = math.sqrt(YRANGE[5] ** 2 - y ** 2)

    layer = []
    point_info.append((r, y, 0.0))
    temp = len(point_info)
    layer.append(temp)
    layer.append(temp)

    for i in range(1, NUMPOINTS):
        theta = 2 * math.pi * i / NUMPOINTS
        x = r * math.cos(theta)
        z = r * math.sin(theta)
        point_info.append((x, y, z))
        layer.append(len(point_info))

    layer.append(temp)
    layer.append(temp)

    layer.append(layer[0])

    layers.append(layer)

def part2(y: float):
    global point_info, layers

    r = math.sqrt(YRANGE[5] ** 2 - y ** 2)
    x0 = math.sqrt(PARAM[0] ** 2 - YRANGE[4] ** 2)
    x1 = math.sqrt(YRANGE[5] ** 2 - YRANGE[4] ** 2)
    z0 = math.sqrt(YRANGE[4] ** 2 - y ** 2)
    theta0 = math.asin(z0 / r)
    theta1 = 2 * math.pi - theta0

    layer = []
    point_info.append((x0, y, z0))
    layer.append(len(point_info))
    point_info.append((x1, y, z0))
    layer.append(len(point_info))

    for i in range(1, NUMPOINTS):
        theta = ((NUMPOINTS - i) * theta0 + i * theta1) / NUMPOINTS
        x = r * math.cos(theta)
        z = r * math.sin(theta)
        point_info.append((x, y, z))
        layer.append(len(point_info))
    
    point_info.append((x1, y, -z0))
    layer.append(len(point_info))
    point_info.append((x0, y, -z0))
    layer.append(len(point_info))

    layer.append(layer[0])
    
    layers.append(layer)

def part3(y: float):
    global point_info, layers

    r = math.sqrt(PARAM[0] ** 2 - y ** 2)
    x0 = math.sqrt(PARAM[0] ** 2 - YRANGE[4] ** 2)
    z0 = math.sqrt(YRANGE[4] ** 2 - y ** 2)
    theta0 = math.asin(z0 / r)
    theta1 = 2 * math.pi - theta0

    layer = []
    point_info.append((x0, y, z0))
    layer.append(len(point_info))
    layer.append(len(point_info))

    for i in range(1, NUMPOINTS):
        theta = ((NUMPOINTS - i) * theta0 + i * theta1) / NUMPOINTS
        x = r * math.cos(theta)
        z = r * math.sin(theta)
        point_info.append((x, y, z))
        layer.append(len(point_info))
    
    point_info.append((x0, y, -z0))
    layer.append(len(point_info))
    layer.append(len(point_info))

    layer.append(layer[0])

    layers.append(layer)

def part4():
    global point_info, layers

    layer = []
    point_info.append((0.0, 0.0, 0.0))
    for i in range(NUMPOINTS + 1):
        layer.append(len(point_info))
    layers.append(layer)

    layer = []
    for i in range(NUMPOINTS):
        theta = 2 * math.pi * i / NUMPOINTS
        x = PARAM[1] * math.cos(theta)
        y = PARAM[1] * math.sin(theta)
        point_info.append((x, y, 0.0))
        layer.append(len(point_info))
    layer.append(layer[0])
    layers.append(layer)

    h = YRANGE[5] - math.sqrt(PARAM[0] ** 2 - YRANGE[4] ** 2)

    layer = []
    for i in range(NUMPOINTS):
        theta = 2 * math.pi * i / NUMPOINTS
        x = PARAM[1] * math.cos(theta)
        y = PARAM[1] * math.sin(theta)
        point_info.append((x, y, h / 2))
        layer.append(len(point_info))
    layer.append(layer[0])
    layers.append(layer)

    layer = []
    for i in range(NUMPOINTS):
        theta = 2 * math.pi * i / NUMPOINTS
        x = PARAM[2] * math.cos(theta)
        y = PARAM[2] * math.sin(theta)
        point_info.append((x, y, h / 2))
        layer.append(len(point_info))
    layer.append(layer[0])
    layers.append(layer)

    layer = []
    for i in range(NUMPOINTS):
        theta = 2 * math.pi * i / NUMPOINTS
        x = PARAM[2] * math.cos(theta)
        y = PARAM[2] * math.sin(theta)
        point_info.append((x, y, h))
        layer.append(len(point_info))
    layer.append(layer[0])
    layers.append(layer)

    layer = []
    point_info.append((0.0, 0.0, h))
    for i in range(NUMPOINTS + 1):
        layer.append(len(point_info))
    layers.append(layer)

def main():
    global point_info, layers
    fn = [part1, part2, part3, part2, part1]

    for i in range(5):
        for y in range(YRANGE[i], YRANGE[i + 1] + 1):
            fn[i](y)
    
    with open('pokeball_body.obj', 'w') as f:
        for x, y, z in point_info:
            f.write('v {} {} {} 1.0\n'.format(x, y, z))
        for i in range(1, len(layers)):
            for j in range(NUMPOINTS + 3):
                f.write('f {} {} {} {}\n'.format(layers[i - 1][j], layers[i - 1][j + 1], layers[i][j + 1], layers[i][j]))

    point_info = []
    layers = []
    part4()
    with open('pokeball_button.obj', 'w') as f:
        for x, y, z in point_info:
            f.write('v {} {} {} 1.0\n'.format(x, y, z))
        for i in range(1, len(layers)):
            for j in range(NUMPOINTS):
                f.write('f {} {} {} {}\n'.format(layers[i - 1][j], layers[i - 1][j + 1], layers[i][j + 1], layers[i][j]))

if __name__ == '__main__':
    main()
