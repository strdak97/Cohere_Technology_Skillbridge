int sum_array(const int *values, size_t count) {
  int totalNum = 0;
  for (int i = 0; i < count; i++) {
    totalNum = totalNum + values[i];
  }
  return totalNum;
}