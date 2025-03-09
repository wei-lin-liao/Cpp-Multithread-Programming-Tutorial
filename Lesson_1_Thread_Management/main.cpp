//------------------------------------------------------------------------------
// 標頭檔說明 / Include Libraries Explanation:
//
// <iostream>  : 提供輸入輸出串流功能，用於 std::cout、std::endl 等.
//              Provides input/output stream functionality.
//
// <thread>    : 提供多執行緒支持，例如 std::thread、std::this_thread 等.
//              Provides multi-threading support.
//
// <vector>    : 提供動態陣列容器，用於儲存多個 thread 物件.
//              Provides dynamic array container (std::vector).
//
// <future>    : 提供非同步操作支援，包括 std::async、std::promise 與 std::future.
//              Provides support for asynchronous operations (std::async, std::promise, std::future).
//
// <chrono>    : 提供計時和時間間隔功能，例如 sleep_for、sleep_until.
//              Provides timing and duration functionalities (sleep_for, sleep_until).
//
// <stdexcept> : 提供標準例外處理類型，例如 std::runtime_error.
//              Provides standard exception types (e.g., std::runtime_error).
//
// <functional>: 提供函式對象與綁定功能（本程式中未直接使用，但常用於高階函式操作）.
//              Provides function objects and binding functionality.
//
// <mutex>     : 提供互斥鎖功能，用於保護共享資源 (如 std::cout).
//              Provides mutex functionality for protecting shared resources.
//------------------------------------------------------------------------------

#include <iostream>
#include <thread>
#include <vector>
#include <future>
#include <chrono>
#include <stdexcept>
#include <functional>
#include <mutex>

// 全域 mutex 用來保護 std::cout
std::mutex cout_mutex;

//======================================================================
// 基本 Thread 建立與執行 / Basic Thread Creation and Execution
//======================================================================

/// --------------------------------------------------------
/// 基本任務函式 / Basic Task Function
/// 此函式使用 <iostream> 輸出訊息，並使用 <chrono> 和 <thread> 模擬工作延遲.
/// This function outputs messages and simulates a work delay.
void basicTask(int id, int value)
{
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[basicTask] Thread id: " << std::this_thread::get_id()
                  << ", Task id: " << id << ", value: " << value << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

//======================================================================
// 輔助函式 / Helper Functions
//======================================================================

/// --------------------------------------------------------
/// 輔助執行緒函式 / Helper Thread Function
/// 用來持續印出訊息，顯示其運作進度.
/// Helper function that prints messages repeatedly.
void helperTask()
{
    for (int i = 0; i < 5; ++i)
    {
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "[Helper] Running, iteration " << i
                      << ", thread id: " << std::this_thread::get_id() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

//======================================================================
// 背景執行緒與 Detach / Background Thread and Detach
//======================================================================

/// --------------------------------------------------------
/// 背景任務函式 / Background Task Function
/// 模擬背景執行緒工作，展示 detach 用法.
/// Demonstrates a background task that runs independently when detached.
void backgroundTask()
{
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[backgroundTask] Background thread (id: "
                  << std::this_thread::get_id() << ") started." << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[backgroundTask] Background thread (id: "
                  << std::this_thread::get_id() << ") finished." << std::endl;
    }
}

//======================================================================
// 例外處理 / Exception Handling in Threads
//======================================================================

/// --------------------------------------------------------
/// 例外處理任務函式 / Exception Task Function
/// 此函式使用 <stdexcept> 拋出例外，並利用 <future> 中的 std::promise 傳遞例外給主執行緒.
/// Intentionally throws an exception and uses a promise to pass it to the main thread.
void exceptionTask(std::promise<void>& prom)
{
    try
    {
        throw std::runtime_error("Exception from exceptionTask");
    }
    catch (...)
    {
        prom.set_exception(std::current_exception());
    }
}

//======================================================================
// 非同步任務分派 / Asynchronous Dispatch using std::async
//======================================================================

/// --------------------------------------------------------
/// 非同步任務函式 / Asynchronous Task Function
/// 此函式使用 <future> 中的 std::async 分派非同步任務，並利用 <chrono> 與 <thread> 模擬計算延遲.
/// Uses std::async to dispatch an asynchronous task and simulates a computation delay.
int asyncTask(int x, int y)
{
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[asyncTask] Running in thread (id: "
                  << std::this_thread::get_id() << ")" << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    return x + y;
}

//======================================================================
// 主程式 / Main Function
//======================================================================
int main()
{
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        unsigned int availableThreads = std::thread::hardware_concurrency();
        std::cout << "[Main] Available hardware threads: " << availableThreads << std::endl;
    }

    // ------ Step 1: 基本 Thread 建立與執行 ------
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "\n[Step 1] Creating a basic thread to run basicTask..." << std::endl;
    }
    
    // 一個新的執行緒被創建
    // 這個新執行緒與主執行緒同時運作，兩者之間是並行的（具體順序由作業系統排程決定）
    // std::thread thread名稱 (執行function, 執行function的參數列表...);
    std::thread t1(basicTask, 1, 100);
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[Step 1] Thread t1 id: " << t1.get_id() << std::endl;
    }
    
    // Join 與 joinable() 的示範
    // true  : t1 仍然關聯著一個有效的執行緒
    //         並且該執行緒
    //         (1) 尚未被 join 或
    //         (2) 尚未被 detach
    //
    // false : (1) 預設建構（即沒有關聯任何執行緒）
    //         (2) 已經被 join() 或 detach() 過
    //         (3) 曾經關聯一個執行緒，但透過移動操作（std::move）後變成空的（moved-from）
    
    // 每個用 std::thread 建立的執行緒
    // 你必須在其物件生命週期結束前呼叫 join 或 detach
    // 否則析構函式會觸發 std::terminate()
    // 這樣可以避免未處理的 joinable 狀態導致程式崩潰。
    if (t1.joinable())
    {
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "[Step 1] t1 is joinable. Joining t1..." << std::endl;
        }
        t1.join();
    }

    // ------ Step 2: Sleep 示範 ------
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "\n[Step 2] Demonstrating sleep_for and sleep_until." << std::endl;
        std::cout << "[Step 2] Main thread sleeping for 200ms using sleep_for..." << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    auto wakeTime = std::chrono::system_clock::now() + std::chrono::milliseconds(100);
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[Step 2] Main thread sleeping until 100ms from now using sleep_until..." << std::endl;
    }
    std::this_thread::sleep_until(wakeTime);

    // ------ Step 3: Yield 示範 ------
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "\n[Step 3] Demonstrating yield with helper threads." << std::endl;
    }
    const int helperCount = 3;
    std::vector<std::thread> helpers;
    for (int i = 0; i < helperCount; ++i)
    {
        helpers.push_back(std::thread(helperTask));
    }
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[Main] Main thread doing some work before yielding." << std::endl;
    }
    for (int i = 0; i < helperCount; ++i)
    {
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "[Main] Work iteration " << i << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[Main] Main thread yielding now..." << std::endl;
    }
    
    // 只是給作業系統一個提示
    // 表示「我願意讓出目前的 CPU 時間片」
    // 但它不保證其他執行緒會立即執行
    // 也不會強制進行執行緒切換
    std::this_thread::yield();
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[Main] Main thread resumed after yield." << std::endl;
    }
    for (auto &th : helpers)
    {
        if (th.joinable())
            th.join();
    }

    // ------ Step 4: Thread Swap 操作 ------
    // 交換各自所代表的底層執行緒（即每個 std::thread 物件所擁有的執行緒 ID、狀態等內部資訊）
    // move 和 swap 主要影響的是在主執行緒中如何管理這些 thread 物件，也就是改變它們的「擁有權」或內部狀態，而不會中斷或改變底層執行緒本身的執行。
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "\n[Step 4] Creating threads t3 and t4 for swap demonstration..." << std::endl;
    }
    std::thread t3(basicTask, 3, 300);
    std::thread t4(basicTask, 4, 400);
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[Step 4] Before swap: t3 id: " << t3.get_id()
                  << ", t4 id: " << t4.get_id() << std::endl;
    }
    t3.swap(t4);
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[Step 4] After swap: t3 id: " << t3.get_id()
                  << ", t4 id: " << t4.get_id() << std::endl;
    }
    if (t3.joinable())
        t3.join();
    if (t4.joinable())
        t4.join();

    // ------ Step 5: 轉移所有權 ( Move Semantics ) ------
    // move 和 swap 主要影響的是在主執行緒中如何管理這些 thread 物件，也就是改變它們的「擁有權」或內部狀態，而不會中斷或改變底層執行緒本身的執行。
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "\n[Step 5] Creating thread t5 for ownership transfer..." << std::endl;
    }
    std::thread t5(basicTask, 5, 500);
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[Step 5] t5 id: " << t5.get_id() << std::endl;
    }
    std::thread t6 = std::move(t5);
    if (!t5.joinable())
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[Step 5] t5 is no longer joinable after moving ownership to t6." << std::endl;
    }
    if (t6.joinable())
    {
        t6.join();
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[Step 5] t6 joined after ownership transfer." << std::endl;
    }

    // ------ Step 6: 背景執行緒與 detach ------
    // 當你建立一個執行緒時，必須在它被銷毀前決定如何管理它：
    // (1) 你可以選擇等待它結束（join）或者
    // (2) 讓它在背景獨立運作（detach）
    // 將一個 thread 物件的內部狀態轉移到另一個 thread 物件中
    // 轉移後，原本的 thread 物件變成非 joinable（通常等同於預設建構的狀態），而新物件則接管了那個執行緒
    
    // 背景執行緒：
    // 當你希望執行緒在背景運作，不需要等它結束，也不需要取得結果（例如記錄日誌或執行某些長時間後台任務），你可以使用 detach。這樣執行緒會獨立運作，其資源在執行緒完成後會自動回收。
    
    // 不需要同步的任務：
    // 如果任務完成後不需要與主程式進行任何同步或回傳資料，使用 detach 可以讓主程式不必等待。
    
    // 從 std::thread 物件的管理角度來說
    // 當程式碼結束該 thread 物件的生命週期時
    // 你必須呼叫 join() 或 detach()
    // 否則該物件的析構函式會因為發現還有 joinable 的執行緒而調用 std::terminate()，導致程式崩潰
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "\n[Step 6] Creating thread t7 for background execution..." << std::endl;
    }
    std::thread t7(backgroundTask);
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[Step 6] t7 id: " << t7.get_id() << " will be detached." << std::endl;
    }
    t7.detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // ------ Step 7: 例外處理示範 ------
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "\n[Step 7] Creating thread t8 for exception handling demonstration..." << std::endl;
    }
    
    // std::promise 和 std::future 是 C++ 中用於非同步操作的兩個配對機制
    // 它們允許在不同執行緒之間傳遞數據或例外狀況。
    
    // 建立一個 promise 物件，用來儲存 void 類型的結果或例外訊息
    std::promise<void> prom;
    
    // 從 promise 中取得對應的 future 物件，主執行緒可藉此等待結果或捕捉例外
    std::future<void> fut = prom.get_future();
    
    // 建立一個新執行緒 t8，並傳入 exceptionTask 函式與 promise 的參考
    // 將處理例外的機制與這個執行緒綁定起來，使得在該執行緒中如果發生例外
    // 就可以透過 prom 傳遞到主執行緒對應的 future 中。
    std::thread t8(exceptionTask, std::ref(prom));

    // 檢查 t8 是否 joinable（即 t8 是否仍然關聯著一個有效的執行緒）
    if (t8.joinable())
        // 若 t8 可 join，則等待 t8 執行緒完成工作
        t8.join();

    try
    {
        // 等待 future 的結果。如果在 t8 執行緒中有例外發生，這裡會重新拋出該例外
        fut.get();
    }
    catch (const std::exception &e)
    {
        // 取得 mutex 鎖以保護 std::cout 輸出，防止與其他執行緒的輸出混亂
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[Step 7] Caught exception from t8: " << e.what() << std::endl;
    }

    // ------ Step 8: 非同步任務分派 ------
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "\n[Step 8] Dispatching asynchronous task using std::async..." << std::endl;
    }
    // 非同步地執行 asyncTask
    // 並通過 asyncResult.get() 在未來取得其返回結果
    // 而不會阻塞主執行緒直到該任務完成
    std::future<int> asyncResult = std::async(std::launch::async, asyncTask, 10, 20);
    int result = asyncResult.get();
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[Step 8] Result from asyncTask: " << result << std::endl;
    }

    // ------ Step 9: 識別執行緒 ------
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "\n[Step 9] Main thread id: " << std::this_thread::get_id() << std::endl;
    }

    return 0;
}
