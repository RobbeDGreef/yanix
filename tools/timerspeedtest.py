import time, os

t = time.time()
os.system("make run")
print("process finished in: ", time.time()-t)