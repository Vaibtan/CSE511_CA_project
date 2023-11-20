import matplotlib.pyplot as plt

# Read data from counters.txt
with open('counters.txt', 'r') as file:
    lines = file.readlines()

# Extract register and memory instruction counts
register_instructions = int(lines[0].split(":")[1].strip())
memory_instructions = int(lines[1].split(":")[1].strip())

# Create a pie chart
labels = ['Register Instructions', 'Memory Instructions']
sizes = [register_instructions, memory_instructions]
colors = ['#ff9999', '#66b3ff']  # Red for register, Blue for memory
explode = (0.1, 0)  # explode the 1st slice (i.e., Register Instructions)

plt.pie(sizes, explode=explode, labels=labels, colors=colors, autopct='%1.1f%%', startangle=90)
plt.axis('equal')  # Equal aspect ratio ensures that the pie chart is circular.

# Show the pie chart
plt.title('RISC-V Instruction Distribution')
plt.show()


def read_log_file(file_path):
    cycles = []
    stalls = []

    with open(file_path, 'r') as file:
        lines = file.readlines()

    for line in lines:
        if line.startswith("Cycle"):
            cycle_number = int(line.split()[1].strip(':'))
            cycles.append(cycle_number)
        elif line.startswith("Total number of stalls:"):
            stall_count = int(line.split(":")[1].strip())
            stalls.append(stall_count)

    return cycles, stalls

def plot_graph(cycles, stalls):
    plt.plot(cycles, stalls, 'o')
    plt.title('Stalls vs Cycles')
    plt.xlabel('Cycles')
    plt.ylabel('Stalls')
    plt.grid(True)
    plt.show()

log_file_path = "LogFile.log"
cycles, stalls = read_log_file(log_file_path)
plot_graph(cycles, stalls)
