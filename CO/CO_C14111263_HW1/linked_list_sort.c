#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;

// Split the linked list into two parts
void splitList(Node *head, Node **firstHalf, Node **secondHalf) {
    asm volatile(
        "mv t0, %2\n"             // t0 = head (鏈表的頭節點)
        "mv t1, t0\n"             // t1 = slow (慢指針)
        "mv t2, t0\n"             // t2 = fast (快指針)
        "mv t3, zero\n"           // t3 = prev (NULL)

        "1:\n"
        "lw t4, 4(t2)\n"          // t4 = fast->next
        "beqz t4, 2f\n"           // 如果 fast->next == NULL，跳到結束
        "lw t5, 4(t4)\n"          // t5 = fast->next->next
        "beqz t5, 2f\n"           // 如果 fast->next->next == NULL，跳到結束

        "mv t3, t1\n"             // prev = slow
        "lw t1, 4(t1)\n"          // slow = slow->next
        "mv t2, t5\n"             // fast = fast->next->next

        "j 1b\n"                  // 繼續迴圈

        "2:\n"
        "beqz t3, 3f\n"           // 如果 prev == NULL，則不需要斷開 (鏈結不足以拆分)
        "sw zero, 4(t3)\n"        // prev->next = NULL，分割兩半

        "3:\n"
        "mv %0, t0\n"             // firstHalf = head
        "mv %1, t1\n"             // secondHalf = slow

        : "=r"(*firstHalf), "=r"(*secondHalf)
        : "r"(head)
        : "t0", "t1", "t2", "t3", "t4", "t5"
    );
}

// Merge two sorted linked lists
Node* mergeSortedLists(Node *a, Node *b) {
    Node *result = NULL;
    Node *tail = NULL;
    
    asm volatile(
        // 初始化
        "mv t0, %1\n"            // t0 = a (指向第一個鏈表)
        "mv t1, %2\n"            // t1 = b (指向第二個鏈表)
        "mv t2, zero\n"          // t2 = result (初始化結果鏈表為 NULL)
        "mv t3, zero\n"          // t3 = tail (初始化尾部為 NULL)

        "1:\n"                    // 循環開始
        "beqz t0, 3f\n"           // 如果 a == NULL，跳到處理 b
        "beqz t1, 3f\n"           // 如果 b == NULL，跳到處理 a

        "lw t4, 0(t0)\n"          // t4 = a->data
        "lw t5, 0(t1)\n"          // t5 = b->data
        "blt t4, t5, 2f\n"        // 如果 a->data < b->data，跳到處理 a

        // b 比 a 小，插入 b 到結果中
        "2:\n"
        "lw t6, 4(t1)\n"          // t6 = b->next
        "sw t1, 4(t3)\n"          // tail->next = b (更新尾部指針)
        "mv t3, t1\n"             // tail = b
        "mv t1, t6\n"             // b = b->next
        "j 1b\n"                  // 跳回循環

        // a 比 b 小，插入 a 到結果中
        "3:\n"
        "lw t6, 4(t0)\n"          // t6 = a->next
        "sw t0, 4(t3)\n"          // tail->next = a (更新尾部指針)
        "mv t3, t0\n"             // tail = a
        "mv t0, t6\n"             // a = a->next

        // 當循環結束後，剩餘的鏈表可能還有元素
        "4:\n"
        "beqz t0, 5f\n"           // 如果 a == NULL，跳到處理 b
        "sw t0, 4(t3)\n"          // tail->next = a
        "j 6f\n"

        "5:\n"
        "beqz t1, 6f\n"           // 如果 b == NULL，跳到結束
        "sw t1, 4(t3)\n"          // tail->next = b

        "6:\n"
        "mv %0, t2\n"             // 返回結果鏈表頭部

        : "=r"(result)            // 輸出變數 (用 result 取代 a0)
        : "r"(a), "r"(b)          // 輸入變數
        : "t0", "t1", "t2", "t3", "t4", "t5", "t6"  // 受影響的暫存器
    );
    
    return result;
}

// Merge Sort function for linked list
Node* mergeSort(Node *head) {
    if (!head || !head->next)
        return head; // Return directly if there is only one node

    Node *firstHalf, *secondHalf;

    printf("Splitting list...\n");
    splitList(head, &firstHalf, &secondHalf); // Split the list into two sublists
    printf("Split finished!\n");

    firstHalf = mergeSort(firstHalf); // Recursively sort the left half
    secondHalf = mergeSort(secondHalf); // Recursively sort the right half
    
    return mergeSortedLists(firstHalf, secondHalf); // Merge the sorted sublists
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
        if (i + 1 < list_size) {
            cur->next = (Node*)malloc(sizeof(Node));
            cur = cur->next;
        }
    }

    fclose(input);

    // Linked list sort
    printf("Start sorting...\n");
    head = mergeSort(head);
    printf("Sorting finished!\n");


    cur = head;
    while (cur) {
        printf("%d ", cur->data);
        asm volatile(
            "lw t0, 4(%0)\n"          // t0 = cur->next (加載 cur 的下一個節點)
            "mv %0, t0\n"             // 更新 cur，指向下一個節點
            : "=r"(cur)               // 更新 cur
            : "0"(cur)                // 輸入 cur
            : "t0"                    // 使用的暫存器
        );
        cur = cur->next;
    }
    printf("\n");
    
    return 0;
}
