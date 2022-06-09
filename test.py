import os
from multiprocessing import Process

N = 4


def process_test(bi):
    total = 1000
    for i in range(total):
        os.system(f'dmon-send p-{bi} {bi} {(i+1)/total*100} > /dev/null')


procs = [Process(target=process_test,
                 args=(i,)) for i in range(N)]

[p.start() for p in procs]
[p.join() for p in procs]
