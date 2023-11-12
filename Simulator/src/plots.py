import matplotlib.pyplot as plt
import time

def plot_counters():
    while True:
        try:
            # Read counters from the file
            with open("counters.txt", "r") as counter_file:
                counters = counter_file.read().split()
                register_counter = int(counters[0])
                memory_counter = int(counters[1])

            # Plot the counters
            plt.plot(register_counter, label="Register Instructions")
            plt.plot(memory_counter, label="Memory Instructions")
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
