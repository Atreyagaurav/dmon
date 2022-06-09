import os
import random
import time
from multiprocessing import Process

N = 4


def process_test(bi):
    total = 100
    for i in range(total):
        os.system(f'dmon-send p-{bi} {bi} {(i+1)/total*100} > /dev/null')
        time.sleep(random.random()/10)


procs = [Process(target=process_test,
                 args=(i,)) for i in range(N)]

[p.start() for p in procs]
[p.join() for p in procs]
