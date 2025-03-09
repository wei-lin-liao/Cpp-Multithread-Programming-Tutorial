# Lesson 1. Thread Management

This lesson covers the fundamental concepts of thread management in C++11. Below are the steps outlining the purpose and key concepts behind each part of the code.  
本課程介紹 C++11 中執行緒管理的基本概念。以下各步驟說明了程式碼設計的目的和主要概念。

---

## Step 1: Basic Thread Creation and Execution  
**目的 / Purpose:**  
- Demonstrate how to create a thread using `std::thread` that executes a specific task.  
  展示如何使用 `std::thread` 建立一個執行特定任務的執行緒。  
- Show how the newly created thread runs concurrently with the main thread.  
  說明新建的執行緒如何與主執行緒並行運行。

**概念 / Concepts:**  
- A thread is initiated by providing a function and its parameters.  
  執行緒的建立是通過提供一個函式及其參數來完成的。  
- Managing the thread's lifecycle (using `join` or `detach`) is crucial to avoid unexpected program termination.  
  管理執行緒的生命週期（使用 `join` 或 `detach`）非常重要，以避免程式因未處理的 joinable 執行緒而終止。  
- Mutexes (e.g., `cout_mutex`) ensure that shared resources (like `std::cout`) are accessed in a thread-safe manner.  
  互斥鎖（例如 `cout_mutex`）確保共享資源（如 `std::cout`）的存取是線程安全的。

---

## Step 2: Sleep Demonstration  
**目的 / Purpose:**  
- Illustrate how to control thread execution timing using `sleep_for` and `sleep_until`.  
  說明如何使用 `sleep_for` 與 `sleep_until` 控制執行緒的運行時間。

**概念 / Concepts:**  
- `sleep_for` pauses a thread for a specified duration (e.g., 200ms).  
  `sleep_for` 使執行緒暫停指定的持續時間（例如 200 毫秒）。  
- `sleep_until` pauses a thread until a specific time point is reached.  
  `sleep_until` 使執行緒暫停直到達到某個特定時間點。  
- These functions are used to simulate work delays and control the execution timing in multi-threaded applications.  
  這些函數可用於模擬工作延遲並在多執行緒應用中控制執行時間。

---

## Step 3: Yield Demonstration with Helper Threads  
**目的 / Purpose:**  
- Show how a thread can voluntarily yield control of the CPU with `std::this_thread::yield`.  
  展示如何通過 `std::this_thread::yield` 讓出 CPU 使用權。  
- Use helper threads that continuously print messages to visualize the effect of yielding.  
  使用持續輸出訊息的輔助執行緒來直觀展示 yield 的效果。

**概念 / Concepts:**  
- `yield` is a hint to the OS scheduler that the current thread is willing to give up its CPU time slice.  
  yield 是向作業系統調度器發出的提示，表明當前執行緒願意讓出 CPU 時間片。  
- It promotes fair scheduling among threads but does not guarantee an immediate context switch.  
  它有助於實現執行緒間的公平調度，但不保證立即切換上下文。  
- Helper threads running concurrently help demonstrate that when the main thread yields, other threads get a chance to run.  
  同時運行的輔助執行緒可顯示出當主執行緒讓出 CPU 時，其他執行緒會獲得運行機會。

---

## Step 4: Thread Swap Operation  
**目的 / Purpose:**  
- Demonstrate how to exchange the management (internal state) of two thread objects using `std::thread::swap`.  
  演示如何使用 `std::thread::swap` 來交換兩個執行緒物件的管理狀態。

**概念 / Concepts:**  
- Swapping thread objects exchanges their associated thread handles without affecting the underlying thread execution.  
  交換執行緒物件會互換它們所管理的執行緒（句柄），而不會影響底層執行緒的執行。  
- This technique is useful for reordering threads or managing them within containers.  
  這個技巧有助於重新排列執行緒或在容器中管理執行緒。

---

## Step 5: Ownership Transfer (Move Semantics)  
**目的 / Purpose:**  
- Show how to transfer thread ownership using `std::move`.  
  演示如何使用 `std::move` 來轉移執行緒的所有權。

**概念 / Concepts:**  
- Moving a thread object transfers its management to another object, leaving the original in a non-joinable state.  
  移動一個執行緒物件會將其管理權轉移到另一個物件，原物件變為非 joinable 狀態。  
- The underlying thread continues to execute normally; only the management is transferred.  
  底層執行緒繼續正常運行，僅僅是管理該執行緒的物件發生了改變。

---

## Step 6: Background Thread and Detach  
**目的 / Purpose:**  
- Demonstrate the use of `detach` to allow a thread to run independently in the background.  
  演示如何使用 `detach` 使執行緒在背景中獨立運行。

**概念 / Concepts:**  
- Detached threads are not joined with the main thread; they run independently and automatically free their resources upon termination.  
  分離的執行緒不會與主執行緒同步等待，它們獨立運行並在結束後自動回收資源。  
- This is useful for tasks that do not require synchronization with the main thread (e.g., logging or background tasks).  
  這適用於不需要與主執行緒同步的任務（如記錄或後台任務）。

---

## Step 7: Exception Handling in Threads  
**目的 / Purpose:**  
- Illustrate how to capture and propagate exceptions from a thread using `std::promise` and `std::future`.  
  演示如何使用 `std::promise` 和 `std::future` 捕捉並傳遞執行緒中的例外。

**概念 / Concepts:**  
- A `std::promise<void>` is used to set a result or an exception from within a thread.  
  使用 `std::promise<void>` 來在執行緒中設定結果或例外。  
- The associated `std::future<void>` allows the main thread to wait for the task to complete and to catch any thrown exceptions.  
  對應的 `std::future<void>` 使主執行緒可以等待任務完成並捕捉例外。  
- This mechanism enables safe error propagation across threads.  
  這個機制能夠實現跨執行緒的安全錯誤傳遞。

---

## Step 8: Asynchronous Dispatch using std::async  
**目的 / Purpose:**  
- Show how to perform asynchronous operations using `std::async` and retrieve the result with a `std::future`.  
  演示如何使用 `std::async` 進行非同步操作，並透過 `std::future` 取得結果。

**概念 / Concepts:**  
- `std::async` launches a task asynchronously in a separate thread and returns a `std::future` that will eventually hold the task's result.  
  `std::async` 會在一個單獨的執行緒中非同步啟動任務，並返回一個最終包含結果的 `std::future`。  
- The main thread can call `get()` on the future to obtain the result when needed, allowing for asynchronous execution without immediate blocking.  
  主執行緒可以在需要時調用 `get()` 來獲取結果，從而實現非同步執行而不會立即阻塞。

---

## Step 9: Thread Identification  
**目的 / Purpose:**  
- Demonstrate how to obtain and display the ID of the main thread.  
  展示如何獲取並顯示主執行緒的 ID。

**概念 / Concepts:**  
- Every thread, including the main thread, has a unique identifier that can be retrieved using `std::this_thread::get_id()`.  
  每個執行緒（包括主執行緒）都有一個唯一的識別符，可以通過 `std::this_thread::get_id()` 獲取。  
- Thread IDs are useful for debugging, logging, and ensuring the correct execution flow in multi-threaded applications.  
  執行緒 ID 對於除錯、日誌記錄以及確認多執行緒應用的正確執行流程非常有用。

