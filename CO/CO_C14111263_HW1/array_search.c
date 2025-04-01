#include <stdio.h>

int arraySearch(int *p_a, int arr_size, int target)
{
    int result = -1;

    asm volatile(
        "li t0, 0\n"              // t0 = 0, 初始化索引為 0
        "1:\n"
        "bge t0, %2, 2f\n"        // 如果 t0 >= arr_size, 跳出循環 (跳到 2)
        "slli t2, t0, 2\n"        // t2 = t0 * 4 (因為 int 是 4 bytes)
        "add t3, %4, t2\n"        // t3 = p_a + t2 (計算正確的記憶體位置)
        "lw t1, 0(t3)\n"          // 讀取 arr[t0]
        "beq t1, %3, 3f\n"        // 如果 t1 == target, 跳轉到 3 (找到了)
        "addi t0, t0, 1\n"        // t0 = t0 + 1, 增加索引
        "j 1b\n"                  // 跳回標籤 1，繼續搜尋
        "3:\n"
        "mv %0, t0\n"             // 把找到的索引 (t0) 存入 result
        "j 4f\n"
        "2:\n"
        "mv %0, %1\n"             // result = -1 (若未找到)
        "4:\n"
        : "=r"(result)
        : "r"(-1), "r"(arr_size), "r"(target), "r"(p_a)
        : "t0", "t1", "t2", "t3", "memory"
    );
    return result;
}

// Main function to test the implementation
int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    if (!input) {
        fprintf(stderr, "Error opening file: %s\n", argv[1]);
        return 1;
    }
    int arr_size;
    fscanf(input, "%d", &arr_size);
    int arr[arr_size];

    // Read integers from input file into the array
    for (int i = 0; i < arr_size; i++) {
        int data;
        fscanf(input, "%d", &data);
        arr[i] = data;
    }
    int target;
    fscanf(input, "%d", &target);
    fclose(input);

    int *p_a = &arr[0];

    int index = arraySearch(p_a, arr_size, target);

    // Print the result
    printf("%d ", index);
    printf("\n");

    return 0;
}
