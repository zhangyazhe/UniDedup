
k = 64
f = 16
u = 10
b = 1

t = 1 / (1 + (2 * f / (k + f - 1) * (u / b)))

time_all_pipe = f / b
time_all_conv = (k + f - 1) / u

time_all_min = min(time_all_conv, time_all_pipe)

time_hybrid = max(2 * t * f / b, (1 - t) * (k + f - 1) / u)

print(time_all_min)
print(time_hybrid)
print("t:" + str(t))
print("rate: " + str((time_all_min - time_hybrid) / time_all_min))