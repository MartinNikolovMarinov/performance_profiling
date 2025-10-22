import matplotlib.pyplot as plt

# NOTE: The results here are nearly the same because they actually measure DRAM speed not the drive speed. Since the
#       test code doesn't actually measure with O_DIRECT for no page cache. That would bring these numbers way down.

def dataset1():
    sizes = [
        512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072,
        262144, 524288, 1048576, 2097152, 4194304, 8388608,
        16777216, 33554432
    ]
    labels = [
        "512 B", "1 KiB", "2 KiB", "4 KiB", "8 KiB", "16 KiB", "32 KiB", "64 KiB",
        "128 KiB", "256 KiB", "512 KiB", "1 MiB", "2 MiB", "4 MiB",
        "8 MiB", "16 MiB", "32 MiB"
    ]
    throughput = [
        1.13, 2.15, 3.50, 5.45, 8.30, 10.50, 12.33, 13.62, 15.14,
        13.50, 14.50, 14.80, 14.32, 13.44, 9.82, 9.01, 8.72
    ]
    return (sizes, labels, throughput)

def dataset2():
    sizes = [
        512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072,
        262144, 524288, 1048576, 2097152, 4194304, 8388608,
        16777216, 33554432
    ]
    labels = [
        "512 B", "1 KiB", "2 KiB", "4 KiB", "8 KiB", "16 KiB", "32 KiB", "64 KiB",
        "128 KiB", "256 KiB", "512 KiB", "1 MiB", "2 MiB", "4 MiB",
        "8 MiB", "16 MiB", "32 MiB"
    ]
    throughput = [
        1.16, 2.18, 3.55, 5.89, 8.24, 10.34, 11.92, 13.18, 13.31,
        12.98, 13.05, 13.14, 13.06, 13.00, 12.10, 8.53, 8.37
    ]
    return (sizes, labels, throughput)

def dataset3():
    sizes = [
        8192, 16384, 32768, 65536, 131072,
        262144, 524288, 1048576, 2097152,
        4194304, 8388608, 16777216, 33554432
    ]
    labels = [
        "8 KiB", "16 KiB", "32 KiB", "64 KiB", "128 KiB",
        "256 KiB", "512 KiB", "1 MiB", "2 MiB",
        "4 MiB", "8 MiB", "16 MiB", "32 MiB"
    ]
    throughput = [
        152.08, 173.36, 184.99, 208.06, 248.04,
        184.16, 215.19, 214.31, 210.34,
        159.81, 166.31, 161.70, 164.80
    ]
    return (sizes, labels, throughput)

def dataset4():
    sizes = [
        8192, 16384, 32768, 65536, 131072,
        262144, 524288, 1048576, 2097152,
        4194304, 8388608, 16777216, 33554432
    ]
    labels = [
        "8 KiB", "16 KiB", "32 KiB", "64 KiB", "128 KiB",
        "256 KiB", "512 KiB", "1 MiB", "2 MiB",
        "4 MiB", "8 MiB", "16 MiB", "32 MiB"
    ]
    # Average throughput values (converted to GB/s where needed)
    throughput = [
        0.406, 0.640, 0.809, 1.008, 1.040,
        1.360, 1.470, 1.480, 1.910,
        2.390, 2.730, 2.810, 3.000
    ]
    units = "GB/s"
    return (sizes, labels, throughput, units)

# Load datasets
sizes1, labels1, t1 = dataset1()
sizes2, labels2, t2 = dataset2()
sizes3, labels3, t3 = dataset3()
sizes4, labels4, t4, units4 = dataset4()

# Plot
plt.figure(figsize=(9, 5))
plt.plot(sizes1, t1, marker='o', linewidth=2, label='SSD Dataset with page caching')
plt.plot(sizes2, t2, marker='s', linewidth=2, label='HDD Dataset with page caching')
plt.plot(sizes4, t4, marker='D', linewidth=2, label='SSD Dataset O_DIRECT')
plt.plot(sizes3, [x / 1000 for x in t3], marker='D', linewidth=2, label='HDD Dataset O_DIRECT')

plt.xscale('log', base=2)
plt.xticks(sizes1, labels1, rotation=45, ha='right')
plt.xlabel('Chunk Size')
plt.ylabel('Throughput (GB/s)')
plt.title('Read Throughput vs. Chunk Size (Comparison)')
plt.grid(True, which='both', linestyle=':', alpha=0.6)
plt.legend()
plt.tight_layout()
plt.show()
