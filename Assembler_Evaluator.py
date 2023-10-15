def compare_files(input_file_path, output_file_path, result_file_path):
    score = 0
    total = 0
    lines = []
    status = []
    remark = ""

    with open(input_file_path, 'r') as input_file, open(output_file_path, 'rb') as output_file, open(result_file_path, 'r') as result_file:
        input = input_file.read().splitlines()
        output = output_file.read()
        result = result_file.read().splitlines()

        total = min(len(input), len(output)//32, len(result))

        for i in range(total):
            output_i = output[i*32 : (i + 1)*32].decode('utf-8')
            if result[i] == output_i:
                score += 1
                remark = "Passed"
            else:
                remark = "Failed"
            lines.append(input[i])
            status.append(remark)
    return score, total, lines, status

input_file_path  = "Input.txt"     # Input file which contains assembly instructions
output_file_path = "Output.bin"    # File generated from "Assembler.py"
result_file_path = "binary.txt"    # File containing the correct conversion of "Input.txt" to binary, used to check "Output.bin"
evaluation_file_path = "Assembler_Evaluation.txt" #Text file storing the detailed evaluation

score, total, lines, status = compare_files(input_file_path, output_file_path, result_file_path)

with open(evaluation_file_path, 'w') as evaluation_file:
    evaluation_file.write(f"Test cases passed = {score}\n")
    evaluation_file.write(f"Test cases failed = {total - score}\n\n")
    evaluation_file.write("Status by line:\n")
    for i in range(total):
        evaluation_file.write(f"Test Case {i}: \t {lines[i].ljust(30)} - {status[i]}\n")


print(f"Test cases passed = {score}")
print(f"Test cases failed = {total - score}")
