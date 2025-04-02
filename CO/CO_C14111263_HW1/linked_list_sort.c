#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;

// Split the linked list into two parts
void splitList(Node *head, Node **firstHalf, Node **secondHalf) {
    asm volatile(
        // 初始化指針
        "mv t0, %2\n"             // t0 = head (鏈表的頭節點)
        "beqz t0, 3f\n"           // 如果 head 為 NULL，直接返回

        "mv t1, t0\n"             // t1 = slow (慢指針)
        "mv t2, t0\n"             // t2 = fast (快指針)
        "mv t3, zero\n"           // t3 = prev (前一個節點，初始化為 NULL)

        "1:\n"
        "lw t4, 4(t2)\n"          // t4 = fast->next
        "beqz t4, 2f\n"           // 如果 fast->next == NULL，跳到結束
        "lw t5, 4(t4)\n"          // t5 = fast->next->next
        "beqz t5, 2f\n"           // 如果 fast->next->next == NULL，跳到結束

        // 更新指針位置
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

// Function to print the linked list
void printList(Node *head) {
    Node *cur = head;
    while (cur) {
        printf("%d ", cur->data);
        cur = cur->next;
    }
    printf("\n");
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

    // Testing splitList function
    Node *firstHalf = NULL, *secondHalf = NULL;
    splitList(head, &firstHalf, &secondHalf);

    // Print the two halves of the linked list
    printf("First half: ");
    printList(firstHalf);
    
    printf("Second half: ");
    printList(secondHalf);

    return 0;
}
