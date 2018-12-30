import os
import random

l = range(534, 534+32);
random.shuffle(l);

with open('in.txt', 'w') as f:
    for number in l:
        f.write("%s\n" % number)

f.close();
