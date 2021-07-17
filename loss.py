import matplotlib.pyplot as plt
import numpy as np
plt.style.use("seaborn")
with open("intermediate.txt","r") as f:
    data=f.readlines()
    x=[]
    y=[]
    z=[]
    for i in range(5,80,5):
        x.append(i)
    for i in data:
        y.append(32.3/float(i.split()[0]))
        z.append(float(i.split()[0]))
plt.ylim(0,40)
plt.xlim(0,80)
plt.plot(x,y)
plt.xlabel("Packet Loss %")
plt.ylabel("Throughput (in kB/sec)")
plt.title("Throughput vs Packet Loss %")
plt.savefig("Throughput vs Packet Loss %", bbox_inches = 'tight')
plt.show()

plt.plot(x,z)
plt.ylim(0,80)
plt.xlim(0,80)
plt.xlabel("Packet Loss %")
plt.ylabel("Time Taken (sec)")
plt.title("Time vs Packet Loss %")
plt.savefig("Time vs Packet Loss %", bbox_inches = 'tight')
plt.show()