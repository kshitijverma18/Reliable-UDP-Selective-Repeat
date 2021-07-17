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
# y=y[::-1]
x=x[::-1]
plt.ylim(10,30)
plt.xlim(0,80)
plt.plot(x,y)
# plt.ylim(10,60)
# plt.xticks(rotation=90)
plt.xlabel("Packets Reorder %")
plt.ylabel("Throughput (in kB/sec)")
plt.title("Throughput vs Packets Reorder % (Delay=40ms)")
plt.savefig("Throughput vs  Packets Reorder %", bbox_inches = 'tight')
plt.show()


plt.ylim(1,2.5)
plt.xlim(0,80)
plt.plot(x,z)
# plt.ylim(10,60)
# plt.xticks(rotation=90)
plt.xlabel("Packets Reorder %")
plt.ylabel("Time Taken (in sec)")
plt.title("Time vs Packets Reorder % (Delay=40ms)")
plt.savefig("Time vs  Packets Reorder %", bbox_inches = 'tight')
plt.show()