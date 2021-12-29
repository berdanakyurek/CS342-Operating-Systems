# Generates an input file

from random import randint

no_of_bursts = 1000
filename = 'input.txt'
burstSizeRange = [1, 100]
biggest_burst_step = 100

prev = 0
file = open(filename, "w")


for i in range(no_of_bursts):
    file.write(str(i+1) + " ")
    if i != 0:
        prev += randint(0, biggest_burst_step)
    file.write(str(prev) + " ")
    file.write(str(randint(burstSizeRange[0], burstSizeRange[1])) + "\n")
    pass

file.close()
