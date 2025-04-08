#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;

// Split the linked list into two parts using inline RISC-V assembly
void splitList(Node *head, Node **firstHalf, Node **secondHalf) {
    if (head == NULL || head->next == NULL) {
        *firstHalf = head;
        *secondHalf = NULL;
        return;
    }

    Node *slow = head;
    Node *fast = head->next;

    asm volatile(
        "1:\n"
        "beqz %[fast], 2f\n"
        "ld %[fast], 8(%[fast])\n"
        "beqz %[fast], 2f\n"
        "ld %[slow], 8(%[slow])\n"
        "ld %[fast], 8(%[fast])\n"
        "j 1b\n"
        "2:\n"
        : [slow] "+r"(slow), [fast] "+r"(fast)
        :
        : "memory"
    );

    *firstHalf = head;
    if (slow) {
        *secondHalf = slow->next;
        slow->next = NULL;
    } else {
        *secondHalf = NULL;
    }
}

// Merge two sorted linked lists using inline assembly
Node *mergeSortedLists(Node *a, Node *b) {
    Node *result = NULL;
    Node **tail = &result;

    while (a && b) {
        int aData = a->data;
        int bData = b->data;

        asm volatile(
            "mv t0, %[aData]\n"
            "mv t1, %[bData]\n"
            "ble t0, t1, 1f\n"

            // *tail = b
            "sd %[b], 0(%[tail])\n"
            // b = b->next
            "ld %[b], 8(%[b])\n"
            // tail = &((*tail)->next)
            "ld t2, 0(%[tail])\n"
            "addi t2, t2, 8\n"
            "mv %[tail], t2\n"
            "j 2f\n"

            "1:\n"
            // *tail = a
            "sd %[a], 0(%[tail])\n"
            // a = a->next
            "ld %[a], 8(%[a])\n"
            // tail = &((*tail)->next)
            "ld t2, 0(%[tail])\n"
            "addi t2, t2, 8\n"
            "mv %[tail], t2\n"

            "2:\n"
            : [a] "+r"(a), [b] "+r"(b), [tail] "+r"(tail)
            : [aData] "r"(aData), [bData] "r"(bData)
            : "t0", "t1", "t2", "memory"
        );
    }

    // Append remaining nodes
    if (a) {
        *tail = a;
    } else {
        *tail = b;
    }

    return result;
}

// Merge Sort function for linked list
Node *mergeSort(Node *head) {
    if (head == NULL || head->next == NULL) {
        return head;
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

    Node *head = (list_size > 0) ? (Node *)malloc(sizeof(Node)) : NULL;
    if (!head && list_size > 0) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    Node *cur = head;
    for (int i = 0; i < list_size; i++) {
        fscanf(input, "%d", &(cur->data));
        if (i + 1 < list_size) {
            cur->next = (Node *)malloc(sizeof(Node));
            if (!cur->next) {
                fprintf(stderr, "Memory allocation failed\n");
                return 1;
            }
        } else {
            cur->next = NULL;
        }
        cur = cur->next;
    }
    fclose(input);

    // Linked list sort
    head = mergeSort(head);
    cur = head;

    // Print the sorted list using inline assembly to move to next node
    while (cur) {
        printf("%d ", cur->data);

        asm volatile(
            "mv t0, %0\n"
            "ld t1, 8(t0)\n"
            "mv %0, t1\n"
            : "=r"(cur)
            : "0"(cur)
            : "t0", "t1", "memory"
        );
    }
    printf("\n");

    return 0;
}
