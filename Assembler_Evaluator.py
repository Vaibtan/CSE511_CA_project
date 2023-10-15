def compare_files(input_file_path, output_file_path, result_file_path):
    score = 0
    total = 0
    lines = []
    status = []
    reamrk = ""

    with open(input_file_path, 'r') as input_file, open(output_file_path, 'r') as output_file, open(result_file_path, 'r') as result_file:
        input = input_file.read().splitlines()
        output = output_file.read().splitlines()
        result = result_file.read().splitlines()

        total = min(len(input), len(output), len(result))

        for i in range(total):
            if result[i] == output[i]:
                score += 1
                remark = "Passed"
            else:
                remark = "Failed"
            lines.append(input[i])
            status.append(remark)
    return score, total, lines, status

input_file_path  = "Input.txt"
output_file_path = "Output.txt"
result_file_path = "binary.txt"
evaluation_file_path = "Assembler_Evaluation.txt"

score, total, lines, status = compare_files(input_file_path, output_file_path, result_file_path)

with open(evaluation_file_path, 'w') as evaluation_file:
    evaluation_file.write(f"Test cases passed = {score}\n")
    evaluation_file.write(f"Test cases failed = {total - score}\n\n")
    evaluation_file.write("Status by line:\n")
    for i in range(total):
        evaluation_file.write(f"{lines[i].ljust(30)} - {status[i]}\n")


print(f"Test cases passed = {score}")
print(f"Test cases failed = {total - score}")