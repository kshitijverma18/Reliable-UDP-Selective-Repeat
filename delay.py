import matplotlib.pyplot as plt
import numpy as np
plt.style.use("seaborn")
with open("intermediate.txt","r") as f:
    data=f.readlines()
    x=[]
    y=[]
    z=[]
    for i in range(100,1501,100):
        x.append(i)
    for i in data:
        y.append(32.3/float(i.split()[0]))
        z.append(float(i.split()[0]))
plt.ylim(0,5)
plt.xlim(0,1600)
plt.plot(x,y)
plt.xticks(rotation=90)
plt.xlabel("Packet Delay (in ms)")
plt.ylabel("Throughput (in kB/sec)")
plt.title("Throughput vs Packet Delay (in ms)")
plt.savefig("Throughput vs Packet Delay (in ms)", bbox_inches = 'tight')
plt.show()

plt.ylim(0,100)
plt.xlim(0,1600)
plt.plot(x,z)
plt.xticks(rotation=90)
plt.xlabel("Packet Delay (in ms)")
plt.ylabel("Time Taken (in sec)")
plt.title("Time vs Packet Delay (in ms)")
plt.savefig("Time vs Packet Delay (in ms)", bbox_inches = 'tight')
plt.show()

