import matplotlib.pyplot as plt
import numpy as np

# Masks and corresponding average bandwidths (GB/s)
data = [
    (0x000003FF, 251.88),
    (0x000007FF, 251.86),
    (0x00000FFF, 251.80),
    (0x00001FFF, 251.87),
    (0x00003FFF, 251.70),
    (0x00007FFF, 243.67),
    (0x0000FFFF, 117.46),
    (0x0001FFFF, 115.49),
    (0x0003FFFF, 95.30),
    (0x0007FFFF, 68.64),
    (0x000FFFFF, 66.73),
    (0x001FFFFF, 66.23),
    (0x003FFFFF, 65.74),
    (0x007FFFFF, 60.72),
    (0x00FFFFFF, 29.78),
    (0x01FFFFFF, 25.88),
    (0x03FFFFFF, 23.45),
    (0x07FFFFFF, 22.91),
    (0x0FFFFFFF, 22.40),
    (0x1FFFFFFF, 22.42),
    (0x3FFFFFFF, 22.27),
    (0x7FFFFFFF, 21.74),
    (0xFFFFFFFF, 21.88),
    (0x1FFFFFFFF, 22.56),
    (0x3FFFFFFFF, 22.70),
    (0x7FFFFFFFF, 22.65),
    (0xFFFFFFFFF, 22.81),
    (0x1FFFFFFFFF, 22.56),
    (0x3FFFFFFFFF, 22.42),
    (0x7FFFFFFFFF, 22.47),
    (0xFFFFFFFFFF, 22.43),
    (0x1FFFFFFFFFF, 22.73),
    (0x3FFFFFFFFFF, 22.55),
    (0x7FFFFFFFFFF, 22.31),
    (0xFFFFFFFFFFF, 22.24),
    (0x1FFFFFFFFFFF, 22.51),
    (0x3FFFFFFFFFFF, 22.64),
    (0x7FFFFFFFFFFF, 22.52),
    (0xFFFFFFFFFFFF, 22.42),
    (0x1FFFFFFFFFFFF, 22.60),
    (0x3FFFFFFFFFFFF, 22.21),
    (0x7FFFFFFFFFFFF, 22.54),
    (0xFFFFFFFFFFFFF, 22.48),
    (0x1FFFFFFFFFFFFF, 22.44),
    (0x3FFFFFFFFFFFFF, 22.60)
]

# Convert to numpy arrays
masks, bandwidth = zip(*data)
sizes = np.array(masks) + 1  # working-set size in bytes

# Convert to KiB for axis readability
sizes_kib = sizes / 1024.0

plt.figure(figsize=(10, 6))
plt.plot(sizes_kib, bandwidth, marker='o', linestyle='-', color='tab:blue')

plt.xscale('log')
plt.xlabel('Working Set Size (KiB, log scale)')
plt.ylabel('Bandwidth (GB/s)')
plt.title('Measured Memory Bandwidth vs Working Set Size')
plt.grid(True, which='both', linestyle='--', linewidth=0.5)

# Annotate cache-level regions roughly for Skylake
plt.axvline(32, color='gray', linestyle=':', label='~L1 limit (32 KiB)')
plt.axvline(256, color='gray', linestyle='--', label='~L2 limit (256 KiB)')
plt.axvline(8192, color='gray', linestyle='-.', label='~L3 limit (8 MiB)')
plt.legend()

plt.tight_layout()
plt.show()
