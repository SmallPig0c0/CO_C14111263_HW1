#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;

// 使用 RISC-V 組合語言拆分鏈結串列
void splitList(Node *head, Node **firstHalf, Node **secondHalf) {
    asm volatile(
        "mv t0, %2\n"             // t0 = head
        "beqz t0, 3f\n"           // if head is NULL, return immediately

        "mv t1, t0\n"             // t1 = slow (slow pointer)
        "mv t2, t0\n"             // t2 = fast (fast pointer)
        "mv t3, zero\n"           // t3 = prev (NULL)

        "1:\n"
        "lw t4, 4(t2)\n"          // t4 = fast->next
        "beqz t4, 2f\n"          // if fast->next == NULL, stop
        "lw t5, 4(t4)\n"         // t5 = fast->next->next
        "beqz t5, 2f\n"          // if fast->next->next == NULL, stop

        "mv t3, t1\n"            // prev = slow
        "lw t1, 4(t1)\n"         // slow = slow->next
        "mv t2, t5\n"            // fast = fast->next->next

        "j 1b\n"                 // continue loop

        "2:\n"
        "beqz t3, 3f\n"          // if prev == NULL, don't break
        "sw zero, 4(t3)\n"       // prev->next = NULL (split list)

        "3:\n"
        "mv %0, t0\n"            // firstHalf = head
        "mv %1, t1\n"            // secondHalf = slow

        : "=r"(*firstHalf), "=r"(*secondHalf)
        : "r"(head)
        : "t0", "t1", "t2", "t3", "t4", "t5"
    );
}

// 輸出鏈結串列
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

    // 測試 splitList
    Node *firstHalf = NULL, *secondHalf = NULL;
    splitList(head, &firstHalf, &secondHalf);

    // 印出 slow 指針當前位置以確認拆分點
    if (secondHalf) {
        printf("Slow pointer at node with value: %d\n", secondHalf->data);
    }

    printf("First half: ");
    printList(firstHalf);
    printf("Second half: ");
    printList(secondHalf);

    return 0;
}
