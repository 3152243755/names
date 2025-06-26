#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <limits.h>

// 物品结构体定义
typedef struct {
    int itemId;       // 物品编号
    double itemWeight;// 物品重量
    double itemValue; // 物品价值
    double valueRatio;// 价值重量比
} KnapsackItem;

// 全局变量用于记录内存占用峰值
size_t memoryPeakUsage = 0;

// 内存分配跟踪函数
void* trackMemoryAllocation(size_t size) {
    void *allocation = malloc(size);
    if (allocation) {
        size_t currentUsage = memoryPeakUsage + size;
        if (currentUsage > memoryPeakUsage) {
            memoryPeakUsage = currentUsage;
        }
    }
    return allocation;
}

// 内存释放跟踪函数
void trackMemoryDeallocation(void *allocation, size_t size) {
    free(allocation);
    memoryPeakUsage -= size;
}

// 生成指定范围内的随机浮点数（保留两位小数）
double generateRandomDouble(double minVal, double maxVal) {
    double valueRange = (maxVal - minVal);
    double divisor = RAND_MAX / valueRange;
    double randomValue = minVal + (rand() / divisor);
    return round(randomValue * 100.0) / 100.0; // 保留两位小数
}

// 生成随机物品数组
KnapsackItem* createRandomItems(int itemCount) {
    KnapsackItem *itemArray = (KnapsackItem*)trackMemoryAllocation(itemCount * sizeof(KnapsackItem));
    if (!itemArray) return NULL;
    
    for (int i = 0; i < itemCount; i++) {
        itemArray[i].itemId = i + 1;
        itemArray[i].itemWeight = generateRandomDouble(1.0, 100.0);    // 1.00~100.00的随机重量
        itemArray[i].itemValue = generateRandomDouble(100.0, 1000.0);  // 100.00~1000.00的随机价值
        itemArray[i].valueRatio = itemArray[i].itemValue / itemArray[i].itemWeight;
    }
    return itemArray;
}

// 输出物品信息到CSV文件
void exportItemsToCsv(KnapsackItem *itemArray, int itemCount, int knapsackCapacity) {
    char fileName[50];
    sprintf(fileName, "背包容量%d_物品数量%d.csv", knapsackCapacity, itemCount);
    
    FILE *filePointer = fopen(fileName, "w");
    if (!filePointer) {
        printf("无法创建CSV文件\n");
        return;
    }
    
    // 写入表头
    fprintf(filePointer, "物品编号,物品重量,物品价值\n");
    
    // 写入物品数据
    for (int i = 0; i < itemCount; i++) {
        fprintf(filePointer, "%d,%.2f,%.2f\n", 
                itemArray[i].itemId, 
                itemArray[i].itemWeight, 
                itemArray[i].itemValue);
    }
    
    fclose(filePointer);
    printf("物品信息已导出到: %s\n\n", fileName);
}

// 打印选中的物品编号
void printSelectedItems(KnapsackItem *itemArray, int *selectionArray, int itemCount) {
    printf("  选中的物品编号: [ ");
    int firstItem = 1;
    for (int i = 0; i < itemCount; i++) {
        if (selectionArray[i]) {
            if (!firstItem) printf(", ");
            printf("%d", itemArray[i].itemId);
            firstItem = 0;
        }
    }
    printf(" ]\n");
}

// 快速排序的分区函数
int partition(KnapsackItem arr[], int low, int high) {
    double pivot = arr[high].valueRatio;
    int i = low - 1;
    
    for (int j = low; j < high; j++) {
        if (arr[j].valueRatio > pivot) {
            i++;
            KnapsackItem temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    
    KnapsackItem temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    
    return i + 1;
}

// 快速排序主函数
void quickSort(KnapsackItem arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

// 蛮力法求解0-1背包问题
void bruteForceKnapsack(KnapsackItem *itemArray, int itemCount, int knapsackCapacity) {
    clock_t startTime = clock();
    memoryPeakUsage = sizeof(KnapsackItem) * itemCount; // 重置内存统计
    
    double maxValue = 0;
    int bestSelection = 0;
    long totalCombinations = (long)1 << itemCount; // 2^n种可能性
    
    // 遍历所有子集
    for (long i = 0; i < totalCombinations; i++) {
        double currentWeight = 0;
        double currentValue = 0;
        
        for (int j = 0; j < itemCount; j++) {
            if (i & (1L << j)) {
                currentWeight += itemArray[j].itemWeight;
                currentValue += itemArray[j].itemValue;
            }
        }
        
        if (currentWeight <= knapsackCapacity && currentValue > maxValue) {
            maxValue = currentValue;
            bestSelection = i;
        }
    }
    
    clock_t endTime = clock();
    double executionTime = ((double)(endTime - startTime)) * 1000 / CLOCKS_PER_SEC;
    
    // 输出结果
    printf("┌───────────────────────────────────────────────────────────┐\n");
    printf("│                    蛮力法求解结果                          │\n");
    printf("├───────────────────────────────────────────────────────────┤\n");
    printf("│ 物品数量: %d, 背包容量: %d                                  │\n", itemCount, knapsackCapacity);
    printf("│ 最大价值: %.2f                                             │\n", maxValue);
    int *selectionArray = (int*)malloc(itemCount * sizeof(int));
    memset(selectionArray, 0, itemCount * sizeof(int));
    for (int j = 0; j < itemCount; j++) {
        if (bestSelection & (1L << j)) {
            selectionArray[j] = 1;
        }
    }
    printSelectedItems(itemArray, selectionArray, itemCount);
    free(selectionArray);
    printf("│ 执行时间: %.2f 毫秒                                         │\n", executionTime);
    printf("│ 内存使用: %zu 字节                                          │\n", memoryPeakUsage);
    printf("└───────────────────────────────────────────────────────────┘\n\n");
}

// 动态规划法求解0-1背包问题
void dynamicProgrammingKnapsack(KnapsackItem *itemArray, int itemCount, int knapsackCapacity) {
    clock_t startTime = clock();
    
    // 将浮点重量转换为整数(乘以100)
    int integerCapacity = (int)(knapsackCapacity * 100);
    memoryPeakUsage = sizeof(double) * (integerCapacity + 1) + sizeof(int) * itemCount + sizeof(KnapsackItem) * itemCount;
    
    // 创建DP表和选择记录数组
    double *dpTable = (double*)trackMemoryAllocation((integerCapacity + 1) * sizeof(double));
    int *selectedItems = (int*)trackMemoryAllocation(itemCount * sizeof(int));
    if (!dpTable || !selectedItems) {
        if (dpTable) trackMemoryDeallocation(dpTable, (integerCapacity + 1) * sizeof(double));
        if (selectedItems) trackMemoryDeallocation(selectedItems, itemCount * sizeof(int));
        return;
    }
    
    // 初始化DP表
    for (int weight = 0; weight <= integerCapacity; weight++) {
        dpTable[weight] = 0;
    }
    memset(selectedItems, 0, itemCount * sizeof(int));
    
    // 动态规划求解
    for (int i = 0; i < itemCount; i++) {
        int integerWeight = (int)(itemArray[i].itemWeight * 100);
        for (int weight = integerCapacity; weight >= integerWeight; weight--) {
            if (dpTable[weight] < dpTable[weight - integerWeight] + itemArray[i].itemValue) {
                dpTable[weight] = dpTable[weight - integerWeight] + itemArray[i].itemValue;
                selectedItems[i] = 1; // 标记物品被选中
            }
        }
    }
    
    double maxValue = dpTable[integerCapacity];
    clock_t endTime = clock();
    double executionTime = ((double)(endTime - startTime)) * 1000 / CLOCKS_PER_SEC;
    
    // 输出结果
    printf("┌───────────────────────────────────────────────────────────┐\n");
    printf("│                  动态规划法求解结果                         │\n");
    printf("├───────────────────────────────────────────────────────────┤\n");
    printf("│ 物品数量: %d, 背包容量: %d                                  │\n", itemCount, knapsackCapacity);
    printf("│ 最大价值: %.2f                                             │\n", maxValue);
    printSelectedItems(itemArray, selectedItems, itemCount);
    printf("│ 执行时间: %.2f 毫秒                                         │\n", executionTime);
    printf("│ 内存使用: %zu 字节                                          │\n", memoryPeakUsage);
    printf("└───────────────────────────────────────────────────────────┘\n\n");
    
    trackMemoryDeallocation(dpTable, (integerCapacity + 1) * sizeof(double));
    trackMemoryDeallocation(selectedItems, itemCount * sizeof(int));
}

// 贪心法求解0-1背包问题（使用快速排序)
void greedyKnapsack(KnapsackItem *itemArray, int itemCount, int knapsackCapacity) {
    clock_t startTime = clock();
    memoryPeakUsage = sizeof(KnapsackItem) * itemCount + sizeof(int) * itemCount;
    
    // 按价值重量比排序 
    KnapsackItem *sortedItems = (KnapsackItem*)trackMemoryAllocation(itemCount * sizeof(KnapsackItem));
    if (!sortedItems) return;
    memcpy(sortedItems, itemArray, itemCount * sizeof(KnapsackItem));
    
    // 快速排序
    quickSort(sortedItems, 0, itemCount - 1);
    
    // 贪心选择
    double currentWeight = 0;
    double totalValue = 0;
    int *selectedItems = (int*)trackMemoryAllocation(itemCount * sizeof(int));
    if (!selectedItems) return;
    memset(selectedItems, 0, itemCount * sizeof(int));
    
    for (int i = 0; i < itemCount; i++) {
        if (currentWeight + sortedItems[i].itemWeight <= knapsackCapacity) {
            selectedItems[i] = 1;
            currentWeight += sortedItems[i].itemWeight;
            totalValue += sortedItems[i].itemValue;
        }
    }
    
    clock_t endTime = clock();
    double executionTime = ((double)(endTime - startTime)) * 1000 / CLOCKS_PER_SEC;
    
    // 输出结果
    printf("┌───────────────────────────────────────────────────────────┐\n");
    printf("│                    贪心法求解结果                          │\n");
    printf("├───────────────────────────────────────────────────────────┤\n");
    printf("│ 物品数量: %d, 背包容量: %d                                  │\n", itemCount, knapsackCapacity);
    printf("│ 最大价值: %.2f                                             │\n", totalValue);
    printSelectedItems(sortedItems, selectedItems, itemCount);
    printf("│ 执行时间: %.2f 毫秒                                         │\n", executionTime);
    printf("│ 内存使用: %zu 字节                                          │\n", memoryPeakUsage);
    printf("└───────────────────────────────────────────────────────────┘\n\n");
    
    trackMemoryDeallocation(sortedItems, itemCount * sizeof(KnapsackItem));
    trackMemoryDeallocation(selectedItems, itemCount * sizeof(int));
}

// 回溯法辅助函数
void backtrackKnapsack(KnapsackItem *itemArray, int itemCount, int knapsackCapacity, int currentItem, 
                       double currentWeight, double currentValue,
                       int *currentSelection, int *bestSelection, 
                       double *bestValue) {
    if (currentItem == itemCount) {
        if (currentValue > *bestValue) {
            *bestValue = currentValue;
            memcpy(bestSelection, currentSelection, itemCount * sizeof(int));
        }
        return;
    }
    
    // 选择当前物品
    if (currentWeight + itemArray[currentItem].itemWeight <= knapsackCapacity) {
        currentSelection[currentItem] = 1;
        backtrackKnapsack(itemArray, itemCount, knapsackCapacity, currentItem + 1, 
                         currentWeight + itemArray[currentItem].itemWeight, 
                         currentValue + itemArray[currentItem].itemValue,
                         currentSelection, bestSelection, bestValue);
        currentSelection[currentItem] = 0;
    }
    
    // 不选择当前物品
    backtrackKnapsack(itemArray, itemCount, knapsackCapacity, currentItem + 1, 
                     currentWeight, currentValue,
                     currentSelection, bestSelection, bestValue);
}

// 回溯法求解0-1背包问题
void backtrackingKnapsack(KnapsackItem *itemArray, int itemCount, int knapsackCapacity) {
    clock_t startTime = clock();
    memoryPeakUsage = sizeof(KnapsackItem) * itemCount + 2 * sizeof(int) * itemCount;
    
    int *currentSelection = (int*)trackMemoryAllocation(itemCount * sizeof(int));
    int *bestSelection = (int*)trackMemoryAllocation(itemCount * sizeof(int));
    double bestValue = 0;
    
    if (!currentSelection || !bestSelection) {
        if (currentSelection) trackMemoryDeallocation(currentSelection, itemCount * sizeof(int));
        if (bestSelection) trackMemoryDeallocation(bestSelection, itemCount * sizeof(int));
        return;
    }
    
    memset(currentSelection, 0, itemCount * sizeof(int));
    memset(bestSelection, 0, itemCount * sizeof(int));
    
    backtrackKnapsack(itemArray, itemCount, knapsackCapacity, 0, 0.0, 0.0, 
                     currentSelection, bestSelection, &bestValue);
    
    clock_t endTime = clock();
    double executionTime = ((double)(endTime - startTime)) * 1000 / CLOCKS_PER_SEC;
    
    // 输出结果
    printf("┌───────────────────────────────────────────────────────────┐\n");
    printf("│                    回溯法求解结果                          │\n");
    printf("├───────────────────────────────────────────────────────────┤\n");
    printf("│ 物品数量: %d, 背包容量: %d                                  │\n", itemCount, knapsackCapacity);
    printf("│ 最大价值: %.2f                                             │\n", bestValue);
    printSelectedItems(itemArray, bestSelection, itemCount);
    printf("│ 执行时间: %.2f 毫秒                                         │\n", executionTime);
    printf("│ 内存使用: %zu 字节                                          │\n", memoryPeakUsage);
    printf("└───────────────────────────────────────────────────────────┘\n\n");
    
    trackMemoryDeallocation(currentSelection, itemCount * sizeof(int));
    trackMemoryDeallocation(bestSelection, itemCount * sizeof(int));
}

// 性能测试函数
void runPerformanceTest(int itemCount, int knapsackCapacity) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("                性能测试开始 - 物品数: %d, 容量: %d                \n", itemCount, knapsackCapacity);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
    
    srand(time(NULL)); // 重置随机种子
    KnapsackItem *itemArray = createRandomItems(itemCount);
    if (!itemArray) {
        printf("内存分配失败，无法创建物品列表\n");
        return;
    }
    
    // 当物品数量为1000且容量为20000时导出CSV
    if (itemCount == 1000 && knapsackCapacity == 20000) {
        exportItemsToCsv(itemArray, itemCount, knapsackCapacity);
    }
    
    // 仅在小规模时运行蛮力法和回溯法
    if (itemCount <= 20) {
        bruteForceKnapsack(itemArray, itemCount, knapsackCapacity);
        backtrackingKnapsack(itemArray, itemCount, knapsackCapacity);
    }
    
    // 动态规划和贪心法处理大规模数据
    if (itemCount <= 50000 && knapsackCapacity <= 200000) {
        dynamicProgrammingKnapsack(itemArray, itemCount, knapsackCapacity);
    }
    
    greedyKnapsack(itemArray, itemCount, knapsackCapacity);
    
    trackMemoryDeallocation(itemArray, itemCount * sizeof(KnapsackItem));
}

int main() {
    printf("===============================================================\n");
    printf("                 0-1背包问题算法性能优化测试                    \n");
    printf("===============================================================\n\n");
    
    // 定义测试规模
    int itemCounts[] = {10, 20, 1000, 5000, 10000, 20000, 30000, 40000, 50000};
    int capacities[] = {10000, 100000, 200000};
    int countCount = sizeof(itemCounts) / sizeof(itemCounts[0]);
    int capacityCount = sizeof(capacities) / sizeof(capacities[0]);
    
    // 运行性能测试
    for (int i = 0; i < capacityCount; i++) {
        for (int j = 0; j < countCount; j++) {
            // 跳过不合理的组合
            if (itemCounts[j] > 50000) continue;
            if (itemCounts[j] > 40000 && capacities[i] == 200000) continue;
            
            runPerformanceTest(itemCounts[j], capacities[i]);
        }
    }
    
    printf("===============================================================\n");
    printf("                          测试完成                             \n");
    printf("===============================================================\n");
    return 0;
}
