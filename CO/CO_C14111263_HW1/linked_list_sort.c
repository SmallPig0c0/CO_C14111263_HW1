#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;

// Block C (Move to the next node)
void moveToNextNode(Node **cur) {
    asm volatile(
        "ld t0, 8(%0)\n"          // t0 = cur->next
        "mv %0, t0\n"             // 更新 cur 為 cur->next
        : "=r"(*cur)               // 更新 cur
        : "0"(*cur)                // 輸入 cur
        : "t0"                     // 使用的暫存器
    );
}

// Block A (splitList) - Split the linked list into two halves
void splitList(Node *head, Node **firstHalf, Node **secondHalf) {
    asm volatile(
        "mv t0, %2\n"             // t0 = head (鏈表頭)
        "beqz t0, 3f\n"           // 如果 head == NULL，跳過操作

        "mv t1, t0\n"             // t1 = slow (slow 指針)
        "mv t2, t0\n"             // t2 = fast (fast 指針)
        "mv t3, zero\n"           // t3 = prev (NULL)

        "1:\n"
        "ld t4, 8(t2)\n"          // t4 = fast->next
        "beqz t4, 2f\n"           // 如果 fast->next == NULL，跳出
        "ld t5, 8(t4)\n"          // t5 = fast->next->next
        "beqz t5, 2f\n"           // 如果 fast->next->next == NULL，跳出

        "mv t3, t1\n"             // prev = slow
        "ld t1, 8(t1)\n"          // slow = slow->next
        "mv t2, t5\n"             // fast = fast->next->next

        "j 1b\n"                  // 繼續循環

        "2:\n"
        "beqz t3, 3f\n"           // 如果 prev == NULL，跳過
        "sd zero, 8(t3)\n"        // prev->next = NULL，斷開鏈表

        "3:\n"
        "mv %0, t0\n"             // firstHalf = head
        "mv %1, t1\n"             // secondHalf = slow

        : "=r"(*firstHalf), "=r"(*secondHalf)
        : "r"(head)
        : "t0", "t1", "t2", "t3", "t4", "t5", "memory"
    );
}

// Block B (mergeSortedLists) - Merge two sorted linked lists into one
Node *mergeSortedLists(Node *a, Node *b) {
    Node *result = NULL;
    Node *tail = NULL;

    asm volatile(
        "mv t0, %1\n"            // t0 = a
        "mv t1, %2\n"            // t1 = b
        "mv t2, zero\n"          // t2 = result (初始化結果為 NULL)
        "mv t3, zero\n"          // t3 = tail (初始化尾部為 NULL)

        "1:\n"
        "beqz t0, 4f\n"           // 如果 a == NULL，跳到處理 b
        "beqz t1, 4f\n"           // 如果 b == NULL，跳到處理 a

        "ld t4, 0(t0)\n"          // t4 = a->data
        "ld t5, 0(t1)\n"          // t5 = b->data
        "blt t4, t5, 2f\n"        // 如果 a->data < b->data，跳到處理 a

        // 處理 b < a
        "ld t6, 8(t1)\n"          // t6 = b->next
        "beqz t2, 3f\n"           // 如果 result == NULL，設置 result = b
        "sd t1, 8(t3)\n"          // tail->next = b
        "j 3f\n"

        "2:\n"                    // 處理 a < b
        "ld t6, 8(t0)\n"          // t6 = a->next
        "beqz t2, 3f\n"           // 如果 result == NULL，設置 result = a
        "sd t0, 8(t3)\n"          // tail->next = a

        "3:\n"
        "mv t3, t0\n"             // tail = a 或 b
        "mv t0, t6\n"             // a = a->next 或 b = b->next
        "j 1b\n"                  // 跳回循環

        // 當一個鏈表空了，接上另一個鏈表
        "4:\n"
        "beqz t0, 5f\n"           // 如果 a == NULL，跳到處理 b
        "sd t0, 8(t3)\n"          // tail->next = a
        "j 6f\n"

        "5:\n"
        "beqz t1, 6f\n"           // 如果 b == NULL，跳到結束
        "sd t1, 8(t3)\n"          // tail->next = b

        "6:\n"
        "mv %0, t2\n"             // 返回結果鏈表頭部

        : "=r"(result)            // 輸出變數
        : "r"(a), "r"(b)          // 輸入變數
        : "t0", "t1", "t2", "t3", "t4", "t5", "t6", "memory"
    );

    return result;
}

// Merge Sort function for linked list
Node *mergeSort(Node *head) {
    if (!head || !head->next) {
        return head;  // Return directly if there is only one node
    }

    Node *firstHalf, *secondHalf;
    
    splitList(head, &firstHalf, &secondHalf);

    firstHalf = mergeSort(firstHalf);
    secondHalf = mergeSort(secondHalf);

    return mergeSortedLists(firstHalf, secondHalf);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    if (!input) {
        fprintf(stderr, "Error opening file: %s\n", argv[1]);
        return 1;
    }

    int list_size;
    fscanf(input, "%d", &list_size);

    Node *head = (list_size > 0) ? (Node*)malloc(sizeof(Node)) : NULL;
    Node *cur = head;

    for (int i = 0; i < list_size; i++) {
        fscanf(input, "%d", &cur->data);
        if (i + 1 < list_size) 
            cur->next = (Node*)malloc(sizeof(Node));
        cur = cur->next;
    }
    fclose(input);

    // Linked list sort
    head = mergeSort(head);
    
    cur = head;
    while (cur) {
        printf("%d ", cur->data);
        moveToNextNode(&cur);  // 使用內聯組合語言移動指針
    }
    printf("\n");

    return 0;
}
