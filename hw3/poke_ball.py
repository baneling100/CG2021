import math

SCALE = 4
YRANGE = [y / SCALE for y in [-385, -143, -27, 27, 143, 385]]
PARAM = [y / SCALE for y in [378, 91, 58]]
NUMSTEP = [8, 8, 2, 8, 8]
NUMCONTROLPOINTS = 32

def part1(y: float):
    r = math.sqrt(YRANGE[5] ** 2 - y ** 2)

    buffer = []
    buffer.append('1.0 0.0\n')
    buffer.append('1.0 0.0\n')
    buffer.append('1.0 0.0\n')
    buffer.append('1.0 0.0\n')
    for i in range(1, NUMCONTROLPOINTS - 7):
        theta = 2 * math.pi * i / (NUMCONTROLPOINTS - 7)
        x = math.cos(theta)
        z = math.sin(theta)
        buffer.append('{:.6f} {:.6f}\n'.format(x, z))
    buffer.append('1.0 0.0\n')
    buffer.append('1.0 0.0\n')
    buffer.append('1.0 0.0\n')
    buffer.append('1.0 0.0\n')
    buffer.append('{:.6f}\n'.format(r))
    buffer.append('0.0 0 0 0\n')
    buffer.append('0 {:.6f} 0\n'.format(y))
    buffer.append('\n')
    
    return buffer

def part2(y: float):
    r = math.sqrt(YRANGE[5] ** 2 - y ** 2)
    x0 = math.sqrt(PARAM[0] ** 2 - YRANGE[4] ** 2) / r
    x1 = math.sqrt(YRANGE[5] ** 2 - YRANGE[4] ** 2) / r
    z0 = math.sqrt(YRANGE[4] ** 2 - y ** 2) / r
    theta0 = math.asin(z0)
    theta1 = 2 * math.pi - theta0

    buffer = []
    buffer.append('{:.6f} {:.6f}\n'.format(x0, z0))
    buffer.append('{:.6f} {:.6f}\n'.format(x0, z0))
    buffer.append('{:.6f} {:.6f}\n'.format(x1, z0))
    buffer.append('{:.6f} {:.6f}\n'.format(x1, z0))
    for i in range(1, NUMCONTROLPOINTS - 7):
        theta = ((NUMCONTROLPOINTS - 7 - i) * theta0 + i * theta1) / (NUMCONTROLPOINTS - 7)
        x = math.cos(theta)
        z = math.sin(theta)
        buffer.append('{:.6f} {:.6f}\n'.format(x, z))
    buffer.append('{:.6f} {:.6f}\n'.format(x1, -z0))
    buffer.append('{:.6f} {:.6f}\n'.format(x1, -z0))
    buffer.append('{:.6f} {:.6f}\n'.format(x0, -z0))
    buffer.append('{:.6f} {:.6f}\n'.format(x0, -z0))
    buffer.append('{:.6f}\n'.format(r))
    buffer.append('0.0 0 0 0\n')
    buffer.append('0 {:.6f} 0\n'.format(y))
    buffer.append('\n')

    return buffer

def part3(y: float):
    r = math.sqrt(PARAM[0] ** 2 - y ** 2)
    x0 = math.sqrt(PARAM[0] ** 2 - YRANGE[4] ** 2) / r
    z0 = math.sqrt(YRANGE[4] ** 2 - y ** 2) / r
    theta0 = math.asin(z0)
    theta1 = 2 * math.pi - theta0

    buffer = []
    buffer.append('{:.6f} {:.6f}\n'.format(x0, z0))
    buffer.append('{:.6f} {:.6f}\n'.format(x0, z0))
    buffer.append('{:.6f} {:.6f}\n'.format(x0, z0))
    buffer.append('{:.6f} {:.6f}\n'.format(x0, z0))
    for i in range(1, NUMCONTROLPOINTS - 7):
        theta = ((NUMCONTROLPOINTS - 7 - i) * theta0 + i * theta1) / (NUMCONTROLPOINTS - 7)
        x = math.cos(theta)
        z = math.sin(theta)
        buffer.append('{:.6f} {:.6f}\n'.format(x, z))
    buffer.append('{:.6f} {:.6f}\n'.format(x0, -z0))
    buffer.append('{:.6f} {:.6f}\n'.format(x0, -z0))
    buffer.append('{:.6f} {:.6f}\n'.format(x0, -z0))
    buffer.append('{:.6f} {:.6f}\n'.format(x0, -z0))
    buffer.append('{:.6f}\n'.format(r))
    buffer.append('0.0 0 0 0\n')
    buffer.append('0 {:.6f} 0\n'.format(y))
    buffer.append('\n')
    
    return buffer

def part4():
    buffer = []

    for i in range(NUMCONTROLPOINTS):
        theta = 2 * math.pi * (i + 0.5) / NUMCONTROLPOINTS
        x = math.cos(theta)
        z = math.sin(theta)
        buffer.append('{:.6f} {:.6f}\n'.format(x, z))
    buffer.append('0.0\n')
    buffer.append('0.0 0 0 0\n')
    buffer.append('0 {:.6f} 0\n'.format(YRANGE[5]))
    buffer.append('\n')

    for i in range(NUMCONTROLPOINTS):
        theta = 2 * math.pi * (i + 0.5) / NUMCONTROLPOINTS
        x = math.cos(theta)
        z = math.sin(theta)
        buffer.append('{:.6f} {:.6f}\n'.format(x, z))
    buffer.append('0.0\n')
    buffer.append('0.0 0 0 0\n')
    buffer.append('0 {:.6f} 0\n'.format(YRANGE[5]))
    buffer.append('\n')

    for i in range(NUMCONTROLPOINTS):
        theta = 2 * math.pi * (i + 0.5) / NUMCONTROLPOINTS
        x = math.cos(theta)
        z = math.sin(theta)
        buffer.append('{:.6f} {:.6f}\n'.format(x, z))
    buffer.append('0.0\n')
    buffer.append('1.570796 0 0 1\n')
    buffer.append('{:.6f} 0 0\n'.format(math.sqrt(PARAM[0] ** 2 - YRANGE[4] ** 2)))
    buffer.append('\n')

    for i in range(NUMCONTROLPOINTS):
        theta = 2 * math.pi * (i + 0.5) / NUMCONTROLPOINTS
        x = math.cos(theta)
        z = math.sin(theta)
        buffer.append('{:.6f} {:.6f}\n'.format(x, z))
    buffer.append('{}\n'.format(PARAM[1]))
    buffer.append('1.570796 0 0 1\n')
    buffer.append('{:.6f} 0 0\n'.format(math.sqrt(PARAM[0] ** 2 - YRANGE[4] ** 2)))
    buffer.append('\n')

    for i in range(NUMCONTROLPOINTS):
        theta = 2 * math.pi * (i + 0.5) / NUMCONTROLPOINTS
        x = math.cos(theta)
        z = math.sin(theta)
        buffer.append('{:.6f} {:.6f}\n'.format(x, z))
    buffer.append('{}\n'.format(PARAM[1]))
    buffer.append('1.570796 0 0 1\n')
    buffer.append('{:.6f} 0 0\n'.format((YRANGE[5] + math.sqrt(PARAM[0] ** 2 - YRANGE[4] ** 2)) / 2))
    buffer.append('\n')

    for i in range(NUMCONTROLPOINTS):
        theta = 2 * math.pi * (i + 0.5) / NUMCONTROLPOINTS
        x = math.cos(theta)
        z = math.sin(theta)
        buffer.append('{:.6f} {:.6f}\n'.format(x, z))
    buffer.append('{}\n'.format(PARAM[2]))
    buffer.append('1.570796 0 0 1\n')
    buffer.append('{:.6f} 0 0\n'.format((YRANGE[5] + math.sqrt(PARAM[0] ** 2 - YRANGE[4] ** 2)) / 2))
    buffer.append('\n')

    for i in range(NUMCONTROLPOINTS):
        theta = 2 * math.pi * (i + 0.5) / NUMCONTROLPOINTS
        x = math.cos(theta)
        z = math.sin(theta)
        buffer.append('{:.6f} {:.6f}\n'.format(x, z))
    buffer.append('{}\n'.format(PARAM[2]))
    buffer.append('1.570796 0 0 1\n')
    buffer.append('{:.6f} 0 0\n'.format(YRANGE[5]))
    buffer.append('\n')

    for i in range(NUMCONTROLPOINTS):
        theta = 2 * math.pi * (i + 0.5) / NUMCONTROLPOINTS
        x = math.cos(theta)
        z = math.sin(theta)
        buffer.append('{:.6f} {:.6f}\n'.format(x, z))
    buffer.append('0.0\n')
    buffer.append('1.570796 0 0 1\n')
    buffer.append('{:.6f} 0 0\n'.format(YRANGE[5]))
    buffer.append('\n')

    return buffer

def main():
    buffer = ['CATMULL_ROM\n', '{}\n'.format(sum(NUMSTEP) + 8), '{}\n'.format(NUMCONTROLPOINTS), '\n']
    fn = [part1, part2, part3, part2, part1]

    for i in range(5):
        for j in range(NUMSTEP[i]):
            y = (YRANGE[i] * (NUMSTEP[i] - 1 - j) + YRANGE[i + 1] * j) / (NUMSTEP[i] - 1)
            buffer += fn[i](y)
    buffer += part4()

    with open('poke_ball.txt', 'w') as f:
        f.writelines(buffer)

if __name__ == '__main__':
    main()