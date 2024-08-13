#include <algorithm>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;

/**
 * 多线程桶排序的初步版本，后续可以优化一版
 */

// 单桶排序函数
void bucket_sort(vector<int> &arr, int min_value, int max_value) {
  int bucket_size = max_value - min_value + 1;
  vector<int> bucket(bucket_size, 0);

  // 计数：统计每个元素的频率
  for (int num : arr) {
    bucket[num - min_value]++;
  }

  // 重构原数组：根据元素频率排序
  int idx = 0;
  for (int i = 0; i < bucket_size; i++) {
    while (bucket[i] > 0) {
      arr[idx++] = i + min_value;
      bucket[i]--;
    }
  }
}

// 并行桶排序主函数
void parallel_bucket_sort(vector<int> &arr, int num_threads) {
  int min_value = *min_element(arr.begin(), arr.end());
  int max_value = *max_element(arr.begin(), arr.end());
  int range = max_value - min_value + 1;
  int bucket_range = (range + num_threads - 1) / num_threads;
  vector<thread> threads;
  vector<vector<int>> buckets(num_threads);

  // 分配数据到各个桶中
  for (int num : arr) {
    int bucket_idx = (num - min_value) / bucket_range;
    buckets[bucket_idx].push_back(num);
  }

  // 为每个桶创建一个线程进行排序
  for (int i = 0; i < num_threads; i++) {
    threads.emplace_back(bucket_sort, ref(buckets[i]),
                         min_value + i * bucket_range,
                         min_value + (i + 1) * bucket_range - 1);
  }

  // 等待所有线程完成排序
  for (auto &th : threads) {
    if (th.joinable()) {
      th.join();
    }
  }

  // 将各桶中的数据合并回原数组
  int idx = 0;
  for (auto &bucket : buckets) {
    for (int num : bucket) {
      arr[idx++] = num;
    }
  }
}

int main() {
  vector<int> arr = {9, 4, 1, 7, 9, 1, 2, 0};
  int num_threads = 4; // 线程数，可根据实际情况调整

  parallel_bucket_sort(arr, num_threads);

  // 打印排序后的数组
  for (int num : arr) {
    cout << num << " ";
  }
  cout << endl;

  return 0;
}
