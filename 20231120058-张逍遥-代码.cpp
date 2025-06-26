#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <limits.h>

// ��Ʒ�ṹ�嶨��
typedef struct {
    int itemId;       // ��Ʒ���
    double itemWeight;// ��Ʒ����
    double itemValue; // ��Ʒ��ֵ
    double valueRatio;// ��ֵ������
} KnapsackItem;

// ȫ�ֱ������ڼ�¼�ڴ�ռ�÷�ֵ
size_t memoryPeakUsage = 0;

// �ڴ������ٺ���
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

// �ڴ��ͷŸ��ٺ���
void trackMemoryDeallocation(void *allocation, size_t size) {
    free(allocation);
    memoryPeakUsage -= size;
}

// ����ָ����Χ�ڵ������������������λС����
double generateRandomDouble(double minVal, double maxVal) {
    double valueRange = (maxVal - minVal);
    double divisor = RAND_MAX / valueRange;
    double randomValue = minVal + (rand() / divisor);
    return round(randomValue * 100.0) / 100.0; // ������λС��
}

// ���������Ʒ����
KnapsackItem* createRandomItems(int itemCount) {
    KnapsackItem *itemArray = (KnapsackItem*)trackMemoryAllocation(itemCount * sizeof(KnapsackItem));
    if (!itemArray) return NULL;
    
    for (int i = 0; i < itemCount; i++) {
        itemArray[i].itemId = i + 1;
        itemArray[i].itemWeight = generateRandomDouble(1.0, 100.0);    // 1.00~100.00���������
        itemArray[i].itemValue = generateRandomDouble(100.0, 1000.0);  // 100.00~1000.00�������ֵ
        itemArray[i].valueRatio = itemArray[i].itemValue / itemArray[i].itemWeight;
    }
    return itemArray;
}

// �����Ʒ��Ϣ��CSV�ļ�
void exportItemsToCsv(KnapsackItem *itemArray, int itemCount, int knapsackCapacity) {
    char fileName[50];
    sprintf(fileName, "��������%d_��Ʒ����%d.csv", knapsackCapacity, itemCount);
    
    FILE *filePointer = fopen(fileName, "w");
    if (!filePointer) {
        printf("�޷�����CSV�ļ�\n");
        return;
    }
    
    // д���ͷ
    fprintf(filePointer, "��Ʒ���,��Ʒ����,��Ʒ��ֵ\n");
    
    // д����Ʒ����
    for (int i = 0; i < itemCount; i++) {
        fprintf(filePointer, "%d,%.2f,%.2f\n", 
                itemArray[i].itemId, 
                itemArray[i].itemWeight, 
                itemArray[i].itemValue);
    }
    
    fclose(filePointer);
    printf("��Ʒ��Ϣ�ѵ�����: %s\n\n", fileName);
}

// ��ӡѡ�е���Ʒ���
void printSelectedItems(KnapsackItem *itemArray, int *selectionArray, int itemCount) {
    printf("  ѡ�е���Ʒ���: [ ");
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

// ��������ķ�������
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

// ��������������
void quickSort(KnapsackItem arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

// ���������0-1��������
void bruteForceKnapsack(KnapsackItem *itemArray, int itemCount, int knapsackCapacity) {
    clock_t startTime = clock();
    memoryPeakUsage = sizeof(KnapsackItem) * itemCount; // �����ڴ�ͳ��
    
    double maxValue = 0;
    int bestSelection = 0;
    long totalCombinations = (long)1 << itemCount; // 2^n�ֿ�����
    
    // ���������Ӽ�
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
    
    // ������
    printf("��������������������������������������������������������������������������������������������������������������������������\n");
    printf("��                    �����������                          ��\n");
    printf("��������������������������������������������������������������������������������������������������������������������������\n");
    printf("�� ��Ʒ����: %d, ��������: %d                                  ��\n", itemCount, knapsackCapacity);
    printf("�� ����ֵ: %.2f                                             ��\n", maxValue);
    int *selectionArray = (int*)malloc(itemCount * sizeof(int));
    memset(selectionArray, 0, itemCount * sizeof(int));
    for (int j = 0; j < itemCount; j++) {
        if (bestSelection & (1L << j)) {
            selectionArray[j] = 1;
        }
    }
    printSelectedItems(itemArray, selectionArray, itemCount);
    free(selectionArray);
    printf("�� ִ��ʱ��: %.2f ����                                         ��\n", executionTime);
    printf("�� �ڴ�ʹ��: %zu �ֽ�                                          ��\n", memoryPeakUsage);
    printf("��������������������������������������������������������������������������������������������������������������������������\n\n");
}

// ��̬�滮�����0-1��������
void dynamicProgrammingKnapsack(KnapsackItem *itemArray, int itemCount, int knapsackCapacity) {
    clock_t startTime = clock();
    
    // ����������ת��Ϊ����(����100)
    int integerCapacity = (int)(knapsackCapacity * 100);
    memoryPeakUsage = sizeof(double) * (integerCapacity + 1) + sizeof(int) * itemCount + sizeof(KnapsackItem) * itemCount;
    
    // ����DP���ѡ���¼����
    double *dpTable = (double*)trackMemoryAllocation((integerCapacity + 1) * sizeof(double));
    int *selectedItems = (int*)trackMemoryAllocation(itemCount * sizeof(int));
    if (!dpTable || !selectedItems) {
        if (dpTable) trackMemoryDeallocation(dpTable, (integerCapacity + 1) * sizeof(double));
        if (selectedItems) trackMemoryDeallocation(selectedItems, itemCount * sizeof(int));
        return;
    }
    
    // ��ʼ��DP��
    for (int weight = 0; weight <= integerCapacity; weight++) {
        dpTable[weight] = 0;
    }
    memset(selectedItems, 0, itemCount * sizeof(int));
    
    // ��̬�滮���
    for (int i = 0; i < itemCount; i++) {
        int integerWeight = (int)(itemArray[i].itemWeight * 100);
        for (int weight = integerCapacity; weight >= integerWeight; weight--) {
            if (dpTable[weight] < dpTable[weight - integerWeight] + itemArray[i].itemValue) {
                dpTable[weight] = dpTable[weight - integerWeight] + itemArray[i].itemValue;
                selectedItems[i] = 1; // �����Ʒ��ѡ��
            }
        }
    }
    
    double maxValue = dpTable[integerCapacity];
    clock_t endTime = clock();
    double executionTime = ((double)(endTime - startTime)) * 1000 / CLOCKS_PER_SEC;
    
    // ������
    printf("��������������������������������������������������������������������������������������������������������������������������\n");
    printf("��                  ��̬�滮�������                         ��\n");
    printf("��������������������������������������������������������������������������������������������������������������������������\n");
    printf("�� ��Ʒ����: %d, ��������: %d                                  ��\n", itemCount, knapsackCapacity);
    printf("�� ����ֵ: %.2f                                             ��\n", maxValue);
    printSelectedItems(itemArray, selectedItems, itemCount);
    printf("�� ִ��ʱ��: %.2f ����                                         ��\n", executionTime);
    printf("�� �ڴ�ʹ��: %zu �ֽ�                                          ��\n", memoryPeakUsage);
    printf("��������������������������������������������������������������������������������������������������������������������������\n\n");
    
    trackMemoryDeallocation(dpTable, (integerCapacity + 1) * sizeof(double));
    trackMemoryDeallocation(selectedItems, itemCount * sizeof(int));
}

// ̰�ķ����0-1�������⣨ʹ�ÿ�������)
void greedyKnapsack(KnapsackItem *itemArray, int itemCount, int knapsackCapacity) {
    clock_t startTime = clock();
    memoryPeakUsage = sizeof(KnapsackItem) * itemCount + sizeof(int) * itemCount;
    
    // ����ֵ���������� 
    KnapsackItem *sortedItems = (KnapsackItem*)trackMemoryAllocation(itemCount * sizeof(KnapsackItem));
    if (!sortedItems) return;
    memcpy(sortedItems, itemArray, itemCount * sizeof(KnapsackItem));
    
    // ��������
    quickSort(sortedItems, 0, itemCount - 1);
    
    // ̰��ѡ��
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
    
    // ������
    printf("��������������������������������������������������������������������������������������������������������������������������\n");
    printf("��                    ̰�ķ������                          ��\n");
    printf("��������������������������������������������������������������������������������������������������������������������������\n");
    printf("�� ��Ʒ����: %d, ��������: %d                                  ��\n", itemCount, knapsackCapacity);
    printf("�� ����ֵ: %.2f                                             ��\n", totalValue);
    printSelectedItems(sortedItems, selectedItems, itemCount);
    printf("�� ִ��ʱ��: %.2f ����                                         ��\n", executionTime);
    printf("�� �ڴ�ʹ��: %zu �ֽ�                                          ��\n", memoryPeakUsage);
    printf("��������������������������������������������������������������������������������������������������������������������������\n\n");
    
    trackMemoryDeallocation(sortedItems, itemCount * sizeof(KnapsackItem));
    trackMemoryDeallocation(selectedItems, itemCount * sizeof(int));
}

// ���ݷ���������
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
    
    // ѡ��ǰ��Ʒ
    if (currentWeight + itemArray[currentItem].itemWeight <= knapsackCapacity) {
        currentSelection[currentItem] = 1;
        backtrackKnapsack(itemArray, itemCount, knapsackCapacity, currentItem + 1, 
                         currentWeight + itemArray[currentItem].itemWeight, 
                         currentValue + itemArray[currentItem].itemValue,
                         currentSelection, bestSelection, bestValue);
        currentSelection[currentItem] = 0;
    }
    
    // ��ѡ��ǰ��Ʒ
    backtrackKnapsack(itemArray, itemCount, knapsackCapacity, currentItem + 1, 
                     currentWeight, currentValue,
                     currentSelection, bestSelection, bestValue);
}

// ���ݷ����0-1��������
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
    
    // ������
    printf("��������������������������������������������������������������������������������������������������������������������������\n");
    printf("��                    ���ݷ������                          ��\n");
    printf("��������������������������������������������������������������������������������������������������������������������������\n");
    printf("�� ��Ʒ����: %d, ��������: %d                                  ��\n", itemCount, knapsackCapacity);
    printf("�� ����ֵ: %.2f                                             ��\n", bestValue);
    printSelectedItems(itemArray, bestSelection, itemCount);
    printf("�� ִ��ʱ��: %.2f ����                                         ��\n", executionTime);
    printf("�� �ڴ�ʹ��: %zu �ֽ�                                          ��\n", memoryPeakUsage);
    printf("��������������������������������������������������������������������������������������������������������������������������\n\n");
    
    trackMemoryDeallocation(currentSelection, itemCount * sizeof(int));
    trackMemoryDeallocation(bestSelection, itemCount * sizeof(int));
}

// ���ܲ��Ժ���
void runPerformanceTest(int itemCount, int knapsackCapacity) {
    printf("����������������������������������������������������������������������������������������������������������������������������\n");
    printf("                ���ܲ��Կ�ʼ - ��Ʒ��: %d, ����: %d                \n", itemCount, knapsackCapacity);
    printf("����������������������������������������������������������������������������������������������������������������������������\n\n");
    
    srand(time(NULL)); // �����������
    KnapsackItem *itemArray = createRandomItems(itemCount);
    if (!itemArray) {
        printf("�ڴ����ʧ�ܣ��޷�������Ʒ�б�\n");
        return;
    }
    
    // ����Ʒ����Ϊ1000������Ϊ20000ʱ����CSV
    if (itemCount == 1000 && knapsackCapacity == 20000) {
        exportItemsToCsv(itemArray, itemCount, knapsackCapacity);
    }
    
    // ����С��ģʱ�����������ͻ��ݷ�
    if (itemCount <= 20) {
        bruteForceKnapsack(itemArray, itemCount, knapsackCapacity);
        backtrackingKnapsack(itemArray, itemCount, knapsackCapacity);
    }
    
    // ��̬�滮��̰�ķ�������ģ����
    if (itemCount <= 50000 && knapsackCapacity <= 200000) {
        dynamicProgrammingKnapsack(itemArray, itemCount, knapsackCapacity);
    }
    
    greedyKnapsack(itemArray, itemCount, knapsackCapacity);
    
    trackMemoryDeallocation(itemArray, itemCount * sizeof(KnapsackItem));
}

int main() {
    printf("===============================================================\n");
    printf("                 0-1���������㷨�����Ż�����                    \n");
    printf("===============================================================\n\n");
    
    // ������Թ�ģ
    int itemCounts[] = {10, 20, 1000, 5000, 10000, 20000, 30000, 40000, 50000};
    int capacities[] = {10000, 100000, 200000};
    int countCount = sizeof(itemCounts) / sizeof(itemCounts[0]);
    int capacityCount = sizeof(capacities) / sizeof(capacities[0]);
    
    // �������ܲ���
    for (int i = 0; i < capacityCount; i++) {
        for (int j = 0; j < countCount; j++) {
            // ��������������
            if (itemCounts[j] > 50000) continue;
            if (itemCounts[j] > 40000 && capacities[i] == 200000) continue;
            
            runPerformanceTest(itemCounts[j], capacities[i]);
        }
    }
    
    printf("===============================================================\n");
    printf("                          �������                             \n");
    printf("===============================================================\n");
    return 0;
}
