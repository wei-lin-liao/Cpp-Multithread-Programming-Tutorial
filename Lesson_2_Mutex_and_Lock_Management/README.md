# Lesson 2. Mutex, Lock Management

This lesson covers the essential concepts of mutex and lock management in C++. The code demonstrates various locking strategies—ranging from coarse-grained to fine-grained approaches—and how to leverage different mutex types (such as `std::mutex` and `std::shared_mutex`) to protect shared resources effectively.  
本課程介紹 C++ 中互斥鎖與鎖管理的核心概念。程式碼展示了從粗粒度到細粒度的鎖策略，以及如何利用不同的鎖類型（例如 `std::mutex` 與 `std::shared_mutex`）來有效保護共享資源。

---

## Template-based Lock Performance Testing

**目的 / Purpose:**  
- • Demonstrate a generic function template that tests lock performance using various mutex types.  
  展示如何利用模板函式測試不同互斥鎖型別的效能。  
- • Use compile-time decision making (`if constexpr` with type traits) to specialize behavior for `std::shared_mutex` versus other mutexes.  
  利用編譯期決策（`if constexpr` 結合型別萃取）來針對 `std::shared_mutex` 與其他鎖型別實現不同的行為。

**概念 / Concepts:**  
- **Generic Programming:**  
  Template functions allow writing one unified test routine that works with any mutex type, enhancing code reusability.  
  模板函式讓我們能撰寫一個通用測試程式，適用於任何互斥鎖型別，提升了程式碼的重用性。  
- **Compile-time Branching:**  
  Using `if constexpr (std::is_same<MutexType, std::shared_mutex>::value)` ensures that the code specific to shared mutexes is compiled only when applicable.  
  利用 `if constexpr (std::is_same<MutexType, std::shared_mutex>::value)`，可以在編譯期根據鎖型別選擇正確的程式碼路徑，僅在使用 `std::shared_mutex` 時編譯對應程式碼。  
- **Lock Types:**  
  The function demonstrates different locking mechanisms—`std::lock_guard`, `std::unique_lock`, and for shared mutexes, `std::shared_lock`—based on whether the operation is read-only or write.  
  程式碼根據操作類型（讀取或寫入）選擇合適的鎖機制，如 `std::lock_guard`、`std::unique_lock`，以及針對 shared_mutex 的 `std::shared_lock`。

---

## Coarse-grained Lock Test

**目的 / Purpose:**  
- • Measure the performance of a coarse-grained locking strategy where a single global mutex protects a shared vector.  
  測試使用單一全局鎖保護共享向量的粗粒度鎖策略的效能。  
- • Understand how a global mutex can become a bottleneck when multiple threads contend for the same lock.  
  了解當多個執行緒爭奪同一個全局鎖時，該策略如何成為效能瓶頸。

**概念 / Concepts:**  
- **Coarse-grained Locking:**  
  A single mutex covers the entire data structure, simplifying synchronization but possibly limiting scalability under high contention.  
  粗粒度鎖策略中，一個互斥鎖保護整個資料結構，實現簡單，但在高競爭情況下可能限制擴展性。  
- **Global Mutex:**  
  The global mutex ensures that only one thread updates the vector at a time, providing a clear but potentially restrictive synchronization model.  
  全局互斥鎖確保在任何時刻只有一個執行緒能更新向量，雖然能保護資料一致性，但可能過於嚴格。

---

## Fine-grained Lock Test with `std::mutex`

**目的 / Purpose:**  
- • Test a fine-grained locking strategy where each element in a vector is protected by its own `std::mutex`.  
  測試細粒度鎖策略：使用 `std::mutex` 為向量中的每個元素建立獨立鎖。  
- • Allow multiple threads to concurrently update different elements, thereby reducing lock contention compared to a global lock.  
  允許多個執行緒同時更新不同元素，相對於全局鎖能大幅降低鎖競爭。

**概念 / Concepts:**  
- **Fine-grained Locking:**  
  Each resource (vector element) is protected by an individual lock, which minimizes interference between threads when accessing different parts of the data.  
  細粒度鎖策略中，每個資源（向量元素）都有自己的鎖，能在不同資料部分被同時訪問時降低相互干擾。  
- **Per-element Mutex:**  
  `std::mutex` per element is efficient in scenarios where each element is accessed independently.  
  為每個元素使用 `std::mutex`，在元素獨立訪問的情況下能提供高效保護。

---

## Performance Comparison and Analysis

**目的 / Purpose:**  
- • Compare the performance results between coarse-grained and fine-grained locking strategies under various workloads (compute-bound and I/O-bound).  
  比較在不同負載情況下（計算密集與 I/O 密集）粗粒度鎖與細粒度鎖策略的效能表現。  
- • Analyze trade-offs in terms of simplicity, scalability, and overhead when choosing the appropriate locking mechanism.  
  分析在選擇鎖策略時，簡單性、擴展性與額外開銷之間的取捨。

**概念 / Concepts:**  
- **Scalability:**  
  Fine-grained locking can reduce contention and improve concurrency, especially when different threads work on independent parts of the data.  
  細粒度鎖能降低鎖競爭，提升併發性，特別是當不同執行緒操作獨立資料部分時。  
- **Overhead:**  
  Although shared locks allow multiple concurrent readers, they may introduce extra overhead compared to simple mutexes when contention is already low.  
  共享鎖雖能允許多個讀者同時訪問，但在競爭已經很低的情況下，可能帶來額外開銷。  
- **Workload Sensitivity:**  
  The optimal locking strategy depends heavily on whether the workload is I/O-bound or compute-bound, and on the frequency of read versus write operations.  
  最適鎖策略取決於工作負載是 I/O 密集還是計算密集，以及讀寫操作的頻率。

---

## Experimental Data

### Writing Operation (Exclusive Lock) Tests / 寫入操作 (獨占鎖) 測試

| Test                                                                 | std::mutex (一般互斥鎖) | std::shared_mutex (exclusive) (共享鎖 (獨占)) |
|----------------------------------------------------------------------|-----------------------|----------------------------------------------|
| Compute-bound Write (lock_guard) / 計算密集 (寫入, lock_guard)         | 0.325673 sec          | 0.682908 sec                                 |
| Compute-bound Write (unique_lock) / 計算密集 (寫入, unique_lock)         | 0.512649 sec          | 0.676423 sec                                 |
| I/O-bound Write (lock_guard) / I/O密集 (寫入, lock_guard)               | 1.301490 sec          | 1.305837 sec                                 |
| I/O-bound Write (unique_lock) / I/O密集 (寫入, unique_lock)               | 1.303704 sec          | 1.301930 sec                                 |

---

### Reading Operation (Shared Lock) Tests / 讀取操作 (共享鎖) 測試

| Test                                                                 | std::mutex (一般互斥鎖) | std::shared_mutex (shared_lock) (共享鎖 (shared_lock)) |
|----------------------------------------------------------------------|-----------------------|-------------------------------------------------------|
| Compute-bound Read (lock_guard/shared_lock) / 計算密集 (讀取, lock_guard/shared_lock)   | 0.487807 sec          | 0.310335 sec                                          |
| Compute-bound Read (unique_lock/shared_lock) / 計算密集 (讀取, unique_lock/shared_lock)   | 0.591327 sec          | 0.311275 sec                                          |
| I/O-bound Read (lock_guard/shared_lock) / I/O密集 (讀取, lock_guard/shared_lock)         | 1.316585 sec          | 0.164586 sec                                          |
| I/O-bound Read (unique_lock/shared_lock) / I/O密集 (讀取, unique_lock/shared_lock)         | 1.313087 sec          | 0.162220 sec                                          |

---

### Fine-grained vs Coarse-grained Lock Tests / 細粒度鎖 vs 粗粒度鎖 性能測試

| Test                                                                  | Lock Type (鎖類型)              | Performance (效能) |
|-----------------------------------------------------------------------|---------------------------------|--------------------|
| Coarse-grained Compute-bound / 粗粒度 (全局鎖) 計算密集                  | Global mutex / 全局鎖           | 0.246971 sec       |
| Coarse-grained I/O-bound / 粗粒度 (全局鎖) I/O密集                        | Global mutex / 全局鎖           | 1.343854 sec       |
| Fine-grained Compute-bound / 細粒度 (每個元素鎖) 計算密集                  | Per-element mutex / 每個元素鎖  | 0.012905 sec       |
| Fine-grained I/O-bound / 細粒度 (每個元素鎖) I/O密集                        | Per-element mutex / 每個元素鎖  | 0.163759 sec       |

---

## Summary
- **I/O-bound Scenarios / I/O 密集情況：**  
  When the read ratio is high, using `std::shared_mutex` with shared locks can significantly accelerate performance because multiple threads can read concurrently. However, in write-only cases (0% read), the advantage is negligible.  
  當讀取操作佔比較高時，使用 `std::shared_mutex` 的共享鎖能大幅提升效能，因為多個執行緒可以同時讀取；但在純寫入（讀取比例 0%）的情況下，優勢不明顯。

- **CPU-bound Scenarios / CPU 密集情況：**  
  For CPU-bound workloads, a high read ratio similarly benefits from shared locks, leading to improved performance. In pure write scenarios, both mutex types perform similarly—with shared_mutex potentially incurring a slight overhead.  
  對於 CPU 密集型工作負載，當讀取比例較高時，共享鎖同樣能提升效能；在純寫入情況下，兩者表現接近，但 shared_mutex 可能因額外開銷略顯遜色。

- **Lock Management (lock_guard vs unique_lock) / 鎖管理（lock_guard 與 unique_lock）：**  
  There is little performance difference between using lock_guard and unique_lock, as unique_lock mainly offers more flexible control rather than a significant speed advantage.  
  lock_guard 與 unique_lock 在效能上差異不大，unique_lock 主要提供更靈活的控制選項，而非顯著的效能提升。

- **Overall Trade-offs / 整體取捨：**  
  The optimal locking strategy depends on the workload type (I/O-bound vs. CPU-bound) and the read/write ratio, underscoring the importance of selecting the right lock type to balance simplicity, scalability, and overhead.  
  最適鎖策略取決於工作負載的類型（I/O 密集或 CPU 密集）以及讀寫比例，這凸顯了根據應用需求選擇適當鎖以平衡簡單性、擴展性與額外開銷的重要性。
