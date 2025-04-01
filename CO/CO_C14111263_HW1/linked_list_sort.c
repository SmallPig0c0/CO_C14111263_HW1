#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;

// Block A - Split the linked list into two parts
void splitList(Node *head, Node **firstHalf, Node **secondHalf) {
    asm volatile(
        // Initialize pointers
        "mv t0, %2\n"            // t0 = head (指向鏈表頭)
        "mv t1, t0\n"            // t1 = slow (慢指針)
        "mv t2, t0\n"            // t2 = fast (快指針)
        "mv t3, zero\n"          // t3 = prev (初始化為 NULL)

        // Loop to split the list using fast and slow pointers
        "1:\n"
        "lw t4, 4(t2)\n"         // t4 = fast->next
        "beqz t4, 2f\n"          // If fast->next == NULL, jump to the end
        "lw t5, 4(t4)\n"         // t5 = fast->next->next
        "beqz t5, 2f\n"          // If fast->next->next == NULL, jump to the end

        // Update slow and fast pointers
        "lw t1, 4(t1)\n"         // slow = slow->next
        "lw t2, 4(t5)\n"         // fast = fast->next->next
        "mv t3, t1\n"            // prev = slow

        "j 1b\n"                 // Jump back to continue the loop

        // End of the loop, split the list
        "2:\n"
        "sw zero, 4(t3)\n"       // prev->next = NULL (disconnect the second half)

        // Set the firstHalf and secondHalf pointers
        "mv %0, t0\n"            // firstHalf = head
        "mv %1, t1\n"            // secondHalf = slow

        : "=r"(*firstHalf), "=r"(*secondHalf)  // Output variables
        : "r"(head)                         // Input variable
        : "t0", "t1", "t2", "t3", "t4", "t5"  // Clobbered registers
    );
}

// Block B - Merge two sorted linked lists
Node *mergeSortedLists(Node *a, Node *b) {
    Node *result = NULL;
    Node *tail = NULL;

    asm volatile (
        // Initialize pointers
        "mv t0, %1\n"            // t0 = a (first list)
        "mv t1, %2\n"            // t1 = b (second list)
        "mv t2, zero\n"          // t2 = result (initialize as NULL)
        "mv t3, zero\n"          // t3 = tail (initialize as NULL)

        // Loop to merge lists
        "1:\n"
        "beqz t0, 3f\n"          // If a == NULL, jump to handle b
        "beqz t1, 3f\n"          // If b == NULL, jump to handle a

        "lw t4, 0(t0)\n"         // t4 = a->data
        "lw t5, 0(t1)\n"         // t5 = b->data
        "blt t4, t5, 2f\n"       // If a->data < b->data, jump to handle a

        // b is smaller, insert b into result
        "2:\n"
        "lw t6, 4(t1)\n"         // t6 = b->next
        "sw t1, 0(t3)\n"         // tail->next = b
        "mv t3, t1\n"            // tail = b
        "mv t1, t6\n"            // b = b->next
        "j 1b\n"                 // Jump back to continue the loop

        // a is smaller, insert a into result
        "3:\n"
        "lw t6, 4(t0)\n"         // t6 = a->next
        "sw t0, 0(t3)\n"         // tail->next = a
        "mv t3, t0\n"            // tail = a
        "mv t0, t6\n"            // a = a->next

        // After loop ends, handle remaining elements in a or b
        "4:\n"
        "beqz t0, 5f\n"          // If a == NULL, handle b
        "sw t0, 0(t3)\n"         // tail->next = a
        "j 6f\n"

        "5:\n"
        "beqz t1, 6f\n"          // If b == NULL, jump to the end
        "sw t1, 0(t3)\n"         // tail->next = b

        "6:\n"
        "mv %0, t2\n"            // Return the merged result list head

        : "=r"(result)           // Output variable
        : "r"(a), "r"(b)         // Input variables
        : "t0", "t1", "t2", "t3", "t4", "t5", "t6"  // Clobbered registers
    );

    return result;
}

// Block C - Move to the next node in the linked list
void moveToNextNode(Node **cur) {
    asm volatile(
        "lw t0, 4(%0)\n"         // t0 = cur->next (load the next node)
        "mv %0, t0\n"            // Update cur to point to the next node
        : "=r"(*cur)             // Output variable
        : "0"(*cur)              // Input variable
        : "t0"                   // Clobbered register
    );
}

// Merge Sort function for linked list
Node *mergeSort(Node *head) {
    if (!head || !head->next)
        return head; // Return directly if there is only one node

    Node *firstHalf, *secondHalf;
    splitList(head, &firstHalf, &secondHalf);  // Split the list into two sublists

    firstHalf = mergeSort(firstHalf);   // Recursively sort the left half
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
    Node *head = (list_size > 0) ? (Node *)malloc(sizeof(Node)) : NULL;
    Node *cur = head;
    for (int i = 0; i < list_size; i++) {
        fscanf(input, "%d", &(cur->data));
        if (i + 1 < list_size)
            cur->next = (Node *)malloc(sizeof(Node));
        cur = cur->next;
    }
    fclose(input);

    // Linked list sort
    head = mergeSort(head);

    cur = head;
    while (cur) {
        printf("%d ", cur->data);
        moveToNextNode(&cur);  // Move to the next node
    }
    printf("\n");

    return 0;
}
