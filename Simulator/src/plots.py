import matplotlib.pyplot as plt
import time

def plot_counters():
    cycles = []
    data_stalls = []
    while True:
        try:
            # Read counters from the file
            with open("counters.txt", "r") as counter_file:
                counters = counter_file.read().split()
                register_counter = int(counters[0])
                memory_counter = int(counters[1])

            with open("logfile.txt", "r") as log_file:
                lines = log_file.readlines()
                for line in lines:
                    if "Data Stalls:" in line:
                        data_stalls.append(int(line.split()[-1]))
                        cycles.append(int(lines[1].split()[-1]))

            # Plot the counters
            plt.plot(register_counter, label="Register Instructions")
            plt.plot(memory_counter, label="Memory Instructions")
            plt.plot(cycles, data_stalls, label="Data Stalls", linestyle="--")
            plt.xlabel("Cycle")
            plt.ylabel("Instruction Count")
            plt.legend()
            plt.title("Instruction Counters Plot")
            plt.show()
            time.sleep(1)  # Adjust the sleep duration as needed
        except FileNotFoundError:
            pass  # Ignore if the file is not found

if __name__ == "__main__":
    plot_counters()
