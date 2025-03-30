#include <stdio.h>

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
    fclose(input);

    int *p_a = &arr[0];

    // array a bubble sort
    /* Original C code segment
    for (int i = 0; i < arr_size - 1; i++) {
        for (int j = 0; j < arr_size - i -1; j++) {
            if (*(p_a + j) > *(p_a + j + 1)) {
                int tmp = *(p_a + j);
                *(p_a + j) = *(p_a + j + 1);
                *(p_a + j + 1) = tmp;
            }
        }
    }
    */

    for (int i = 0; i < arr_size - 1; i++) {
        for (int j = 0; j < arr_size - i - 1; j++) {
            asm volatile(
                // Your code
                "lw t0, 0(%0)\n"        // 加載 arr[j] 到 t0
                "lw t1, 0(%1)\n"        // 加載 arr[j+1] 到 t1
                "bge t0, t1, 1f\n"      // 如果 arr[j] >= arr[j+1]，跳過交換
                "sw t1, 0(%0)\n"        // 交換: arr[j] = arr[j+1]
                "sw t0, 0(%1)\n"        // 交換: arr[j+1] = arr[j]
                "1:"
                :
                : "r"(p_a + j), "r"(p_a + j + 1)  // 讀取 arr[j] 和 arr[j+1] 的指標
                : "t0", "t1", "memory"             // 受影響的暫存器
            
            );
        }
    }
    p_a = &arr[0];
    for (int i = 0; i < arr_size; i++)
        printf("%d ", *p_a++);
    printf("\n");
    return 0;
}
