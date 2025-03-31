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

![輸出結果](image.png)
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

# 開發日記：Linked-List Merge Sort Search

## 開發過程
###  1.定義list切割函式 `splitList`
 **目標** ：`splitList`函數的目標是將給定的List分割為兩個部分，並將這兩個部分的指針返回給 `firstHalf` 和 `secondHalf`。

#### 解釋程式 :  
(以9 -> 5 -> 8 -> 3 -> 4 -> 10 -> 1 -> 6 -> 2 -> 7 -> NULL為例)
在這個鏈表中，`head` 指向 9，我們將進行以下操作來分割鏈表。


 **初始化指針**：

```asm
"mv t0, %0\n"  // t0 = head 
"mv t1, t0\n"  // t1 = slow 
"mv t2, t0\n"  // t2 = fast 
"mv t3, zero\n"// t3 = prev (初始化為 NULL)
```  
`t0`, `t1`, `t2` 都指向 `head`，即鏈表的起始位置 9。`t3` 初始化為 `NULL`，它將用來跟蹤 `slow` 指針的前一個節點。

**快慢指針法**：
slow 每次移動一步，fast 每次移動兩步，slow會指在鏈表中間位置。

```asm
"1:\n"  // 循環開始
"lw t4, 0(t2)\n"  // t4 = fast->next 
"lw t5, 0(t4)\n"  // t5 = fast->next->next 
"beqz t5, 2f\n"   // 如果 fast->next->next == NULL，跳出循環
```  
這段代碼取得 `fast->next` 和 `fast->next->next`，並檢查是否為 `NULL`。如果是 `NULL`，說明 `fast` 指針已經遍歷完所有節點，循環結束。

**分割**
```asm
"sw zero, 0(t3)\n"  // prev->next = NULL 
"mv %1, t0\n"       // firstHalf = head
"mv %2, t1\n"       // secondHalf = slow
``` 
最終結果：  
`firstHalf`: 9 -> 5 -> 8 -> 3 -> 4 -> NULL   
`secondHalf`: 10 -> 1 -> 6 -> 2 -> 7 -> NULL



###  2.定義 ` *mergeSortedLists `
**目標**：合併兩個已經排序好的鏈表。

#### 解釋程式 : 
**初始化**：`t0`被設為 `a`，`t1` 被設為 `b`，`t2` 和 `t3` 被設為 `NULL`，用於指向合併後的鏈表頭部和尾部。  
```asm
    "mv t0, %0\n"                   // t0 = a (指向鏈表 a)                 
    "mv t1, %1\n"                   // t1 = b (指向鏈表 b)                   
    "mv t2, zero\n"                 // t2 = result (初始化NULL)             
    "mv t3, zero\n"                 // t3 = tail (初始化NULL)
```


**循環比較**：`t4` 和 `t5` 用來存儲 `a` 和 `b` 的當前節點數據，並進行比較，將較小的放入結果鏈表的尾部。
```asm
// 進入循環比較兩個鏈表的元素
    "1:\n"
    "beqz t0, 3f\n"              // 如果 a == NULL，跳到處理剩餘部分
    "beqz t1, 3f\n"              // 如果 b == NULL，跳到處理剩餘部分

// 比較 a 和 b 的值，將較小的節點插入到結果鏈表
    "lw t4, 0(t0)\n"             // t4 = a->data
    "lw t5, 0(t1)\n"             // t5 = b->data
    "blt t4, t5, 2f\n"           // 如果 a->data < b->data，跳到處理 a 的情況

// b 比 a 小，插入 b 到結果中
    "2:\n"
    "lw t6, 4(t1)\n"             // t6 = b->next
    "sw t1, 0(t3)\n"             // tail->next = b
    "mv t3, t1\n"                // tail = b
    "mv t1, t6\n"                // b = b->next
    "j 1b\n"                     // 跳回循環

// a 比 b 小，插入 a 到結果中
    "3:\n"
    "lw t6, 4(t0)\n"             // t6 = a->next
    "sw t0, 0(t3)\n"             // tail->next = a
    "mv t3, t0\n"                // tail = a
    "mv t0, t6\n"                // a = a->next
```

**剩餘元素**：直接將剩餘的鏈表連接到結果鏈表的尾部。  
```asm
// 當循環結束後，剩餘的鏈表可能還有元素
    "4:\n"
    "beqz t0, 5f\n"              // 如果 a == NULL，跳到處理 b
    "sw t0, 0(t3)\n"             // tail->next = a
    "j 6f\n"

    "5:\n"
    "beqz t1, 6f\n"              // 如果 b == NULL，跳到結束
    "sw t1, 0(t3)\n"             // tail->next = b

```

**返回結果**：回傳`t2`(鏈表頭部)
```asm
"6:\n"
"mv %0, t2\n"                // 返回結果鏈表頭部

: "=r"(result)               // 輸出結果鏈表
: "r"(a), "r"(b)             // 輸入鏈表 a 和 b
: "t0", "t1", "t2", "t3", "t4", "t5", "t6"  // 使用的暫存器
```
 ###  3.打印每個節點數據 
 ```asm   
 // 初始化 cur
    "mv t0, %0\n"             // t0 = cur (指向鏈表頭節點)
        
// 循環開始
    "1:\n"
    "beqz t0, 2f\n"           // 如果 cur 為 NULL，跳到循環結束
        
// 打印 cur->data
    "lw t1, 0(t0)\n"          // t1 = cur->data (加載 cur 的數據)
    "li a0, 1\n"              // 設置 printf 的輸出格式為整數
    "ecall\n"                 // 調用系統服務，打印 t1（即 cur->data）
        
// 更新 cur，指向下一個節點
    "lw t2, 4(t0)\n"          // t2 = cur->next (加載 cur 的下一個節點指針)
    "mv t0, t2\n"             // 更新 cur，指向下一個節點
        
    "j 1b\n"                  // 跳回循環開始
        
// 循環結束
    "2:\n"
```






## 虛擬機常用指令

### 1.將 GitHub 上的檔案複製進虛擬機
```bash
git clone <GitHub_網址>  
```
### 2.評分
```bash
cd ~/riscv/CO_C14111263_HW1
make judge 
```
