# 開發日記：RISC-V  Array Bubble Sort

## 1. 簡介
本作業實作了一個 **使用 RISC-V 內嵌組合語言 (`inline assembly`)** 來執行氣泡排序 (Bubble Sort) 的 C 程式，在 **RISC-V 架構** 下操作記憶體、處理數據，並進行基本的排序運算。

---

## 2. 開發流程

### 2.1 讀取輸入數據
1. 透過 `fscanf()` 讀取輸入檔案的 **數據長度 (`arr_size`)**。
2. 建立長度為 `arr_size` 的 **整數陣列 (`arr[]`)**。
3. 迴圈讀取 `arr_size` 個數字，並存入 `arr[]`。

#### **程式碼段：**
```c
fscanf(input, "%d", &arr_size);
int arr[arr_size];
for (int i = 0; i < arr_size; i++) {
    fscanf(input, "%d", &arr[i]);
}
```

---

### 2.2 Bubble Sort
氣泡排序透過兩層迴圈來進行數據比較與交換。
 
   - **載入數據 (`lw`)**：將 `arr[j]` 和 `arr[j+1]` 分別載入 `t0` 和 `t1`。
   - **比較 (`bge`)**：若 `arr[j] >= arr[j+1]`，則跳過交換。(錯誤)
    <mark> 應該是前一項小於等於跳過交換</mark>。
   - **交換 (`sw`)**：若 `arr[j] < arr[j+1]`，則交換兩數的位置。

#### **程式碼段 (RISC-V 組合語言)**
```c
asm volatile (
    "lw t0, 0(%0)\n"        // 載入 arr[j] 到 t0
    "lw t1, 0(%1)\n"        // 載入 arr[j+1] 到 t1
    "bge t0, t1, 1f\n"    //若 arr[j] >= arr[j+1]，跳過交換 
    "sw t1, 0(%0)\n"        // arr[j] = arr[j+1]
    "sw t0, 0(%1)\n"        // arr[j+1] = arr[j]
    "1:"
    :
    : "r"(p_a + j), "r"(p_a + j + 1)
    : "t0", "t1", "memory"
);
```


---
![image](https://hackmd.io/_uploads/HyFaZLf0Je.png)
> **圖 1**: (錯誤)排列順序倒反 

**Debug**： 將` "bge t0, t1, 1f\n" `改為 `"ble t0, t1, 1f\n"`


### 2.3 輸出排序結果
1. **重設指標** `p_a = &arr[0]`。
2. **逐個印出** 排序後的 `arr[]` 數據。

#### **程式碼段：**
```c
p_a = &arr[0];
for (int i = 0; i < arr_size; i++)
    printf("%d ", *p_a++);
printf("\n");
```

---

# 開發日記：Array Search


## 目標
本程式的目標是實現一個基於 RISC-V 的陣列搜尋函式，並在其中使用內嵌組合語言來進行搜索。該程式讀取一個檔案，該檔案包含一個整數陣列，並查找目標數字在陣列中的索引位置。如果目標數字不存在於陣列中，返回 -1。

## 開發過程

###  定義陣列搜尋函式 `arraySearch`
使用 C 語言編寫了一個線性搜尋函式(**時間複雜度O(n)**)，並在該函式中嵌入了 RISC-V 組合語言來實現搜尋過程。
### 標籤與功能描述

 **標籤 1：迴圈 (t0 從 0 到 arr_size)**
   - 從索引 0 開始，尋找陣列直到 `arr_size`。
   - 檢查是否已經達到陣列的末尾。
   - 找到目標則跳轉到標籤3

 **標籤 2：未找到目標**
   - 如果循環結束，表示未找到目標，將結果設為 -1。

 **標籤 3：找到目標**
   - 如果當前元素等於目標，則跳轉到此標籤，並將找到的索引存入 `result`。

 **標籤 4：結束，`result = t0`**
   - 將找到的目標索引 `t0` 存入 `result`，並結束函式。
   

```c
int arraySearch(int *p_a, int arr_size, int target) {
    int result = -1;  // 預設為 -1，表示未找到目標值
    
    asm volatile (
        "li t0, 0\n"              // t0 = 0, 初始化索引為 0
        "1:\n"                    // 標籤 1，開始循環
        "bge t0, %2, 2f\n"        // 如果 t0 >= arr_size, 跳出循環 (跳到 2)
        "lw t1, 0(%0)\n"          // 加載 p_a[t0] (陣列元素) 到 t1 
        "beq t1, %3, 3f\n"        // 如果 t1 == target, 跳轉到 3 (找到了)
        "addi t0, t0, 1\n"        // t0 = t0 + 1, 增加索引
        "j 1b\n"                  // 跳回標籤 1，繼續搜尋
        "3:\n"                    // 標籤 3，表示找到目標
        "mv %0, t0\n"             // 把找到的索引 (t0) 存入 result
        "j 4f\n"                  // 跳到結束標籤 4
        "2:\n"                    // 標籤 2，表示未找到目標
        "mv %0, %1\n"             // result = -1 (若未找到)
        "4:\n"                    // 結束標籤
        : "=r"(result)            // 輸出：結果存入 result
        : "r"(-1), "r"(arr_size), "r"(target), "r"(p_a)  // 輸入：-1, arr_size, target, p_a
        : "t0", "t1", "memory"    // 影響的暫存器
    );
    
    return result;
}
```
**Debug**：需添加偏移計算(`int`佔四個字節)
```asm
" slli t2, t0, 2\n"        // t2 = t0 * 4 (因為 int 是 4 bytes)
" add t3, %4, t2\n"        // t3 = p_a + t2 (計算正確的記憶體位置)
```

# 開發日記：Linked-List Merge Sort Search

## 開發過程
###  1.定義list切割函式 `splitList`
 **目標** ：`splitList`函數的目標是將給定的List分割為兩個部分，並將這兩個部分的指針返回給 `firstHalf` 和 `secondHalf`。

#### 解釋程式 :  
(以9 -> 5 -> 8 -> 3 -> 4 -> 10 -> 1 -> 6 -> 2 -> 7 -> NULL為例)
在這個鏈表中，`head` 指向 9，我們將進行以下操作來分割鏈表。


 **初始化指針**：

```c
Node *slow = head;
Node *fast = head->next;
```  
`t0`, `t1`, `t2` 都指向 `head`，即鏈表的起始位置 9。`t3` 初始化為 `NULL`，它將用來跟蹤 `slow` 指針的前一個節點。

**快慢指針法**：
slow 每次移動一步，fast 每次移動兩步，slow會指在鏈表中間位置。

```asm
"1:\n"                          // 標記 loop 開始處（label 1）
"beqz %[fast], 2f\n"           // 如果 fast 為 NULL（等於 0），跳到 2 結束迴圈
"ld %[fast], 8(%[fast])\n"     // fast = fast->next；載入 fast 節點的 next 指標
"beqz %[fast], 2f\n"           // 如果 fast == NULL，再跳到 2 結束迴圈
"ld %[slow], 8(%[slow])\n"     // slow = slow->next；慢指標往前一步
"ld %[fast], 8(%[fast])\n"     // fast = fast->next；快指標再往前一步（總共前進兩步）
"j 1b\n"                        // 回到 label 1，繼續迴圈
"2:\n"                          // 標記 loop 結束處（label 2）
```  
這段代碼取得 `fast->next` 和 `fast->next->next`，並檢查是否為 `NULL`。如果是 `NULL`，說明 `fast` 指針已經遍歷完所有節點，循環結束。

**分割**
```c
*firstHalf = head;
if (slow) {
    *secondHalf = slow->next;
    slow->next = NULL;
} else {
    *secondHalf = NULL;
}
``` 
最終結果：  
`firstHalf`: 9 -> 5 -> 8 -> 3 -> 4 -> NULL   
`secondHalf`: 10 -> 1 -> 6 -> 2 -> 7 -> NULL



###  2.定義 ` *mergeSortedLists `
**目標**：合併兩個已經排序好的鏈表。

#### 解釋程式 : 
1. **變數宣告**:
   - `Node *result = NULL;`：建立一個空的鏈結串列 `result`，用來儲存合併後的結果。
   - `Node **tail = &result;`：指標 `tail` 指向 `result`，用來跟蹤鏈結串列的尾端。

2. **主迴圈 (`while (a && b)`)**:
   - 當 `a` 和 `b` 都不是空指標時，執行合併操作。
   - 取出 `a` 和 `b` 節點的資料：`int aData = a->data;` 和 `int bData = b->data;`。

3. **內嵌組合語言部分**:
   - 使用內嵌組合語言進行節點比較和合併。組合語言的目的主要是操作鏈結串列中的指標和資料。
   - 先將 `aData` 和 `bData` 放入暫存器 `t0` 和 `t1`，然後使用 `ble` 指令比較兩者的大小。
   - 根據比較結果，將較小的節點插入到 `tail` 指向的節點後，並更新 `tail` 和相應的鏈結串列指標。

4. **尾端處理**:
   - 在 `a` 和 `b` 其中一個串列遍歷完成後，將剩餘的另一個串列直接連接到 `tail`。


```c
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
```

 ###  3.打印每個節點數據 
 ```asm   
"mv t0, %0\n"
"ld t1, 8(t0)\n"
"mv %0, t1\n"
: "=r"(cur)
: "0"(cur)
: "t0", "t1", "memory"
```
---
**Debug**：

| 差異點       | 第一版（錯誤）                                                                 | 第二版（正確）                                                                  |
|--------------|----------------------------------------------------------------------------------|----------------------------------------------------------------------------------|
| **data 比較** | 使用 `lw` 直接從指標位置讀取資料，若指標為 NULL 可能造成 crash                        | 在 C 程式中先取得 `a->data` 和 `b->data`，再將值傳入組合語言中比較，避免 NULL |
| **迴圈控制** | 嘗試在組合語言中處理完整的邏輯流程，邏輯複雜、易出錯                                | 使用 C 控制 while 迴圈，組合語言僅處理節點選擇與指標移動，更穩定               |
| **NULL 檢查** | 缺少對 `slow` 為 NULL 的處理，在 `splitList` 中可能導致 segmentation fault         | 加入 `if (slow)` 判斷，確保安全地設定 `secondHalf` 與切斷鏈結                    |
| **記憶體分配** | 在建立節點時若 `list_size == 0` 可能導致記憶體錯誤                               | 正確地檢查 `list_size > 0` 並處理 NULL，確保不會多配或漏配記憶體                |
| **list 結尾** | 忽略在最後一個節點設定 `cur->next = NULL`，導致潛在無窮迴圈或 crash                | 正確設定 `cur->next = NULL`，確保 linked list 有正確結尾                         |





## 虛擬機常用指令

### 1.將 GitHub 上的檔案複製進虛擬機
```bash
git clone <GitHub_網址>  
```
### 2.評分
```bash
cd ~/riscv/CO_C14111263_HW1
make judge 
judge -c judge3.conf -v 1
```
## 最終結果
![輸出結果](https://hackmd.io/_uploads/rko_bLG0kg.png)
