//------------------------------------------------------------------------------
// 標頭檔說明 / Include Libraries Explanation:
//
// <iostream>      : 提供輸入輸出串流功能，用於 std::cout、std::endl 等.
//                    Provides input/output stream functionality.
//
// <thread>        : 提供多執行緒支持，例如 std::thread、std::this_thread 等.
//                    Provides multi-threading support.
//
// <mutex>         : 提供互斥鎖功能，用於保護共享資源（例如 std::mutex、std::recursive_mutex、std::timed_mutex).
//                    Provides mutex functionality for protecting shared resources.
//
// <shared_mutex>  : 提供共享互斥鎖 (std::shared_mutex)，支援共享與獨占鎖定（C++17）.
//                    Provides shared mutex supporting shared and exclusive locking (C++17).
//
// <chrono>        : 提供計時與時間間隔功能，例如 sleep_for、sleep_until.
//                    Provides timing and duration functionalities.
//
// <vector>        : 提供動態陣列容器，用於儲存多個 thread 物件等.
//                    Provides dynamic array container (std::vector).
//
// <atomic>        : 提供原子操作類別，用於跨執行緒同步計數.
//                    Provides atomic operations for thread-safe counters.
//
// <iomanip>       : 提供格式化輸出功能，例如 std::setw、std::setprecision.
//                    Provides formatting manipulators.
//------------------------------------------------------------------------------
#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <vector>
#include <atomic>
#include <iomanip>

//===================================================================
// 測試函式 / Testing Function
//===================================================================

/// -----------------------------------------------------------------
///   mtx           ：待測試的互斥鎖物件
///   numThreads    ：使用的執行緒數量
///   iterations    ：每個執行緒在臨界區內執行的迭代次數
///   ioBound       ：若為 true，模擬 I/O 操作（休眠 100 微秒）；否則為計算密集
///   useUniqueLock ：若為 true，使用 std::unique_lock；否則使用 std::lock_guard
///   readOnly      ：若為 true，表示讀取操作（共享鎖），否則為寫入操作（獨占鎖）
template<typename MutexType>
double testLockPerformance(MutexType& mtx, int numThreads, int iterations, bool ioBound, bool useUniqueLock, bool readOnly)
{
    long long sharedCounter = 0;                  // 共享計數器，供所有執行緒存取
    std::atomic<int> readyCount(0);                // 記錄就緒執行緒數量
    bool startFlag = false;                        // 全局開始旗標，所有執行緒等待此旗標
    std::vector<std::thread> threads;              // 儲存所有執行緒的向量
    threads.reserve(numThreads);

    // 建立執行緒
    for (int t = 0; t < numThreads; ++t)
    {
        // 這段 lambda 函數就是每個新執行緒的入口點
        threads.emplace_back([&]()
        {
            readyCount.fetch_add(1);             // 標記自己已就緒
            while (!startFlag)                   // 等待開始訊號
            {
                std::this_thread::yield();
            }
            
            // 若 Mutex 為 std::shared_mutex 且 readOnly 為 true，使用 shared_lock 共享鎖定；否則使用獨占鎖定
            // 在編譯期就能決定哪個分支會被編譯的條件判斷語句
            // (1) 條件成立，則該區塊內的程式碼會被編譯
            // (2) 不成立，則完全不會編譯該區塊
            // 避免在執行階段產生不必要的分支檢查
            
            // 編譯期檢查 MutexType 是否與 std::shared_mutex 相同
            if constexpr (std::is_same<MutexType, std::shared_mutex>::value)
            {
                if (readOnly)
                {
                    for (int i = 0; i < iterations; ++i)
                    {
                        std::shared_lock<std::shared_mutex> lock(mtx);  // 使用 shared_lock 以共享模式鎖定（讀取）
                        if (ioBound)
                            std::this_thread::sleep_for(std::chrono::microseconds(100));  // 模擬 I/O 延遲
                        volatile long long dummy = sharedCounter;         // 模擬讀取操作
                    }
                }
                else
                {
                    for (int i = 0; i < iterations; ++i)
                    {
                        if (useUniqueLock)
                        {
                            std::unique_lock<std::shared_mutex> lock(mtx);  // 使用 unique_lock 鎖定（獨占）
                            if (ioBound)
                                std::this_thread::sleep_for(std::chrono::microseconds(100));
                            ++sharedCounter;  // 寫入操作
                        }
                        else
                        {
                            std::lock_guard<std::shared_mutex> lock(mtx);   // 使用 lock_guard 鎖定（獨占）
                            if (ioBound)
                                std::this_thread::sleep_for(std::chrono::microseconds(100));
                            ++sharedCounter;
                        }
                    }
                }
            }
            else {
                // 對於其他 mutex 類型（例如 std::mutex），僅支援獨占鎖定
                for (int i = 0; i < iterations; ++i)
                {
                    if (useUniqueLock)
                    {
                        std::unique_lock<MutexType> lock(mtx);
                        if (ioBound)
                            std::this_thread::sleep_for(std::chrono::microseconds(100));
                        if (!readOnly)
                            ++sharedCounter;
                        else
                            volatile long long dummy = sharedCounter;
                    }
                    else
                    {
                        std::lock_guard<MutexType> lock(mtx);
                        if (ioBound)
                            std::this_thread::sleep_for(std::chrono::microseconds(100));
                        if (!readOnly)
                            ++sharedCounter;
                        else
                            volatile long long dummy = sharedCounter;
                    }
                }
            }
        });
    }
    
    // 等待所有執行緒準備就緒
    while (readyCount.load() < numThreads)
    {
        std::this_thread::yield();
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();  // 記錄開始時間
    startFlag = true;  // 發送開始訊號
    for (auto& th : threads)
    {
        th.join();   // 等待所有執行緒完成
    }
    auto endTime = std::chrono::high_resolution_clock::now();  // 記錄結束時間
    double sec = std::chrono::duration<double>(endTime - startTime).count();  // 計算總耗時（秒）
    return sec;
}

//===================================================================
// 粗粒度鎖測試 / Coarse-grained Lock Test
//===================================================================

/// -----------------------------------------------------------------
/// 測試向量更新性能（粗粒度鎖）
/// 粗粒度鎖：所有執行緒共用一把鎖
double testCoarseGrainedVectorPerformance(int numThreads, int iterations, int dataSize, bool ioBound)
{
    std::vector<int> data(dataSize, 0);         // 建立一個大小為 dataSize 的向量，初始值為 0
    std::mutex globalMutex;                     // 全局鎖
    std::atomic<int> readyCount(0);             // 執行緒就緒計數器
    bool startFlag = false;
    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    auto threadFunc = [&]()
    {
        readyCount.fetch_add(1);
        while (!startFlag) { std::this_thread::yield(); }
        for (int i = 0; i < iterations; i++)
        {
            std::lock_guard<std::mutex> lock(globalMutex);
            int index = i % dataSize;           // 使用取模選擇索引
            data[index]++;                      // 更新該索引的數值
            if (ioBound)
                std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    };

    for (int i = 0; i < numThreads; i++)
        threads.emplace_back(threadFunc);
    while (readyCount.load() < numThreads) { std::this_thread::yield(); }
    auto startTime = std::chrono::high_resolution_clock::now();
    startFlag = true;
    for (auto &th : threads) th.join();
    auto endTime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(endTime - startTime).count();
}

//===================================================================
// 細粒度鎖測試：每個向量元素都有自己的鎖 / Fine-grained Lock Test (Using std::mutex)
//===================================================================

/// -----------------------------------------------------------------
/// 測試向量更新性能（細粒度鎖）
/// 細粒度鎖：每個元素擁有一把獨立的鎖，允許多個執行緒同時更新不同元素
double testFineGrainedVectorPerformance(int numThreads, int iterations, int dataSize, bool ioBound)
{
    std::vector<int> data(dataSize, 0);                   // 建立資料向量
    std::vector<std::mutex> locks(dataSize);              // 為向量中每個元素建立一把鎖
    std::atomic<int> readyCount(0);
    bool startFlag = false;
    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    auto threadFunc = [&]()
    {
        readyCount.fetch_add(1);
        while (!startFlag)
             { std::this_thread::yield(); }
        for (int i = 0; i < iterations; i++)
        {
            int index = i % dataSize;                     // 選擇更新的索引
            {
                std::lock_guard<std::mutex> lock(locks[index]);  // 僅鎖定該元素的鎖
                data[index]++;
            }
            if (ioBound)
                std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    };

    for (int i = 0; i < numThreads; i++)
        threads.emplace_back(threadFunc);

    while (readyCount.load() < numThreads)
        { std::this_thread::yield(); }

    auto startTime = std::chrono::high_resolution_clock::now();
    startFlag = true;
    for (auto &th : threads)
         th.join();
    auto endTime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(endTime - startTime).count();
}

//===================================================================
// 細粒度鎖測試：每個向量元素都有自己的鎖 (使用 std::shared_mutex)
//===================================================================

/// -----------------------------------------------------------------
/// 測試向量更新性能（細粒度鎖 - 使用 shared_mutex）
/// 細粒度鎖：每個元素擁有一把獨立的 shared_mutex
double testFineGrainedVectorPerformanceShared(int numThreads, int iterations, int dataSize, bool ioBound)
{
    std::vector<int> data(dataSize, 0);                           // 建立資料向量
    std::vector<std::shared_mutex> locks(dataSize);               // 為向量中每個元素建立一把 shared_mutex
    std::atomic<int> readyCount(0);
    bool startFlag = false;
    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    auto threadFunc = [&]()
    {
        readyCount.fetch_add(1);
        while (!startFlag)
        {
            std::this_thread::yield();
        }
        for (int i = 0; i < iterations; i++)
        {
            int index = i % dataSize;                     // 選擇更新的索引
            {
                std::lock_guard<std::shared_mutex> lock(locks[index]);  // 使用 lock_guard 搭配 shared_mutex（獨占鎖定）
                data[index]++;
            }
            if (ioBound)
                std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    };

    for (int i = 0; i < numThreads; i++)
        threads.emplace_back(threadFunc);

    while (readyCount.load() < numThreads)
    {
        std::this_thread::yield();
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    startFlag = true;
    for (auto &th : threads)
         th.join();
    auto endTime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(endTime - startTime).count();
}

int main()
{
    int numThreads = 8;         // 執行緒數量 / Number of threads
    int iterations = 100000;    // 計算密集模式（寫入）的迭代次數 / Iterations per thread for compute-bound write
    int ioIterations = 1000;    // I/O 密集模式（寫入）的迭代次數 / Iterations per thread for I/O-bound write
    int readIterations = 100000;  // 計算密集模式（讀取）的迭代次數 / Iterations per thread for compute-bound read
    int ioReadIterations = 1000;  // I/O 密集模式（讀取）的迭代次數 / Iterations per thread for I/O-bound read

    // 建立兩種鎖：std::mutex（僅支援獨占）與 std::shared_mutex（支援共享讀取）
    // Create two mutex types: std::mutex (exclusive only) and std::shared_mutex (supports shared locking)
    std::mutex mtx;
    std::shared_mutex shrdMtx;

    // 輸出格式設定 / Output formatting settings
    const int widthLabel = 50;
    const int widthTime  = 12;
    std::cout << std::fixed << std::setprecision(6);

    // ------ 寫入操作 測試 / Writing Operation Tests (Exclusive Lock Tests) ------
    std::cout << "\n=== Writing Operation (Exclusive Lock) Tests / 寫入操作 (獨占鎖) 測試 ===\n\n";

    // ------ 計算密集模式（寫入，lock_guard） / Compute-bound Write using lock_guard
    double timeMutexWrite = testLockPerformance(mtx, numThreads, iterations, false, false, false);
    double timeSharedWrite = testLockPerformance(shrdMtx, numThreads, iterations, false, false, false);
    std::cout << std::setw(widthLabel) << "Compute-bound Write (lock_guard) / 計算密集 (寫入, lock_guard):\n";
    std::cout << std::setw(widthLabel) << "  std::mutex / 一般互斥鎖:" << std::setw(widthTime) << timeMutexWrite << " sec\n";
    std::cout << std::setw(widthLabel) << "  std::shared_mutex (exclusive) / 共享鎖 (獨占):" << std::setw(widthTime) << timeSharedWrite << " sec\n\n";

    // ------ 計算密集模式（寫入，unique_lock） / Compute-bound Write using unique_lock ------
    double timeMutexWriteUL = testLockPerformance(mtx, numThreads, iterations, false, true, false);
    double timeSharedWriteUL = testLockPerformance(shrdMtx, numThreads, iterations, false, true, false);
    std::cout << std::setw(widthLabel) << "Compute-bound Write (unique_lock) / 計算密集 (寫入, unique_lock):\n";
    std::cout << std::setw(widthLabel) << "  std::mutex / 一般互斥鎖:" << std::setw(widthTime) << timeMutexWriteUL << " sec\n";
    std::cout << std::setw(widthLabel) << "  std::shared_mutex (exclusive) / 共享鎖 (獨占):" << std::setw(widthTime) << timeSharedWriteUL << " sec\n\n";

    // ------ I/O 密集模式（寫入，lock_guard） / I/O-bound Write using lock_guard ------
    double timeMutexIOWrite = testLockPerformance(mtx, numThreads, ioIterations, true, false, false);
    double timeSharedIOWrite = testLockPerformance(shrdMtx, numThreads, ioIterations, true, false, false);
    std::cout << std::setw(widthLabel) << "I/O-bound Write (lock_guard) / I/O密集 (寫入, lock_guard):\n";
    std::cout << std::setw(widthLabel) << "  std::mutex / 一般互斥鎖:" << std::setw(widthTime) << timeMutexIOWrite << " sec\n";
    std::cout << std::setw(widthLabel) << "  std::shared_mutex (exclusive) / 共享鎖 (獨占):" << std::setw(widthTime) << timeSharedIOWrite << " sec\n\n";

    // ------ I/O 密集模式（寫入，unique_lock） / I/O-bound Write using unique_lock ------
    double timeMutexIOWriteUL = testLockPerformance(mtx, numThreads, ioIterations, true, true, false);
    double timeSharedIOWriteUL = testLockPerformance(shrdMtx, numThreads, ioIterations, true, true, false);
    std::cout << std::setw(widthLabel) << "I/O-bound Write (unique_lock) / I/O密集 (寫入, unique_lock):\n";
    std::cout << std::setw(widthLabel) << "  std::mutex / 一般互斥鎖:" << std::setw(widthTime) << timeMutexIOWriteUL << " sec\n";
    std::cout << std::setw(widthLabel) << "  std::shared_mutex (exclusive) / 共享鎖 (獨占):" << std::setw(widthTime) << timeSharedIOWriteUL << " sec\n";

    // ------ 讀取操作 測試 / Reading Operation Tests (Shared Lock Tests) ------
    std::cout << "\n=== Reading Operation (Shared Lock) Tests / 讀取操作 (共享鎖) 測試 ===\n\n";

    // ------ 計算密集模式（讀取，lock_guard/shared_lock） / Compute-bound Read using lock_guard/shared_lock ------
    double timeMutexRead = testLockPerformance(mtx, numThreads, readIterations, false, false, true);
    double timeSharedRead = testLockPerformance(shrdMtx, numThreads, readIterations, false, false, true);
    std::cout << std::setw(widthLabel) << "Compute-bound Read (lock_guard/shared_lock) / 計算密集 (讀取, lock_guard/shared_lock):\n";
    std::cout << std::setw(widthLabel) << "  std::mutex / 一般互斥鎖:" << std::setw(widthTime) << timeMutexRead << " sec\n";
    std::cout << std::setw(widthLabel) << "  std::shared_mutex (shared_lock) / 共享鎖 (shared_lock):" << std::setw(widthTime) << timeSharedRead << " sec\n\n";

    // ------ 計算密集模式（讀取，unique_lock/shared_lock） / Compute-bound Read using unique_lock/shared_lock ------
    double timeMutexReadUL = testLockPerformance(mtx, numThreads, readIterations, false, true, true);
    double timeSharedReadUL = testLockPerformance(shrdMtx, numThreads, readIterations, false, true, true);
    std::cout << std::setw(widthLabel) << "Compute-bound Read (unique_lock/shared_lock) / 計算密集 (讀取, unique_lock/shared_lock):\n";
    std::cout << std::setw(widthLabel) << "  std::mutex / 一般互斥鎖:" << std::setw(widthTime) << timeMutexReadUL << " sec\n";
    std::cout << std::setw(widthLabel) << "  std::shared_mutex (shared_lock) / 共享鎖 (shared_lock):" << std::setw(widthTime) << timeSharedReadUL << " sec\n\n";

    // ------ I/O 密集模式（讀取，lock_guard/shared_lock） / I/O-bound Read using lock_guard/shared_lock ------
    double timeMutexIORead = testLockPerformance(mtx, numThreads, ioReadIterations, true, false, true);
    double timeSharedIORead = testLockPerformance(shrdMtx, numThreads, ioReadIterations, true, false, true);
    std::cout << std::setw(widthLabel) << "I/O-bound Read (lock_guard/shared_lock) / I/O密集 (讀取, lock_guard/shared_lock):\n";
    std::cout << std::setw(widthLabel) << "  std::mutex / 一般互斥鎖:" << std::setw(widthTime) << timeMutexIORead << " sec\n";
    std::cout << std::setw(widthLabel) << "  std::shared_mutex (shared_lock) / 共享鎖 (shared_lock):" << std::setw(widthTime) << timeSharedIORead << " sec\n\n";

    // ------ I/O 密集模式（讀取，unique_lock/shared_lock） / I/O-bound Read using unique_lock/shared_lock ------
    double timeMutexIOReadUL = testLockPerformance(mtx, numThreads, ioReadIterations, true, true, true);
    double timeSharedIOReadUL = testLockPerformance(shrdMtx, numThreads, ioReadIterations, true, true, true);
    std::cout << std::setw(widthLabel) << "I/O-bound Read (unique_lock/shared_lock) / I/O密集 (讀取, unique_lock/shared_lock):\n";
    std::cout << std::setw(widthLabel) << "  std::mutex / 一般互斥鎖:" << std::setw(widthTime) << timeMutexIOReadUL << " sec\n";
    std::cout << std::setw(widthLabel) << "  std::shared_mutex (shared_lock) / 共享鎖 (shared_lock):" << std::setw(widthTime) << timeSharedIOReadUL << " sec\n";

    // ------ 細粒度鎖 vs 粗粒度鎖 測試 / Fine-grained vs Coarse-grained Lock Tests ------
    int dataSize = 1000;         // 向量大小 / Vector size
    int vecIterations = 100000;  // 向量更新迭代次數（計算密集）/ Iterations per thread for compute-bound vector update
    int ioVecIterations = 1000;  // 向量更新迭代次數（I/O密集）/ Iterations per thread for I/O-bound vector update

    std::cout << "\n=== Fine-grained vs Coarse-grained Lock Tests / 細粒度鎖 vs 粗粒度鎖 性能測試 ===\n\n";

    // ------ 粗粒度鎖測試（全局鎖）------
    double coarseCompute = testCoarseGrainedVectorPerformance(numThreads, vecIterations, dataSize, false);
    double coarseIO = testCoarseGrainedVectorPerformance(numThreads, ioVecIterations, dataSize, true);
    std::cout << std::setw(widthLabel) << "Coarse-grained (Global Mutex) Compute-bound / 粗粒度 (全局鎖) 計算密集:\n";
    std::cout << std::setw(widthLabel) << "  Global mutex / 全局鎖:" << std::setw(widthTime) << coarseCompute << " sec\n";
    std::cout << std::setw(widthLabel) << "Coarse-grained (Global Mutex) I/O-bound / 粗粒度 (全局鎖) I/O密集:\n";
    std::cout << std::setw(widthLabel) << "  Global mutex / 全局鎖:" << std::setw(widthTime) << coarseIO << " sec\n\n";

    // ------ 細粒度鎖測試（每個元素一把鎖，使用 std::mutex）------
    double fineCompute = testFineGrainedVectorPerformance(numThreads, vecIterations, dataSize, false);
    double fineIO = testFineGrainedVectorPerformance(numThreads, ioVecIterations, dataSize, true);
    std::cout << std::setw(widthLabel) << "Fine-grained (Per-element Mutex) Compute-bound / 細粒度 (每個元素鎖) 計算密集:\n";
    std::cout << std::setw(widthLabel) << "  Per-element mutex / 每個元素鎖:" << std::setw(widthTime) << fineCompute << " sec\n";
    std::cout << std::setw(widthLabel) << "Fine-grained (Per-element Mutex) I/O-bound / 細粒度 (每個元素鎖) I/O密集:\n";
    std::cout << std::setw(widthLabel) << "  Per-element mutex / 每個元素鎖:" << std::setw(widthTime) << fineIO << " sec\n\n";

    return 0;   // 程式結束 / End program
}
