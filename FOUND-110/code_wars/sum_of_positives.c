int positive_sum(const int *values, size_t count) {
  int sum = 0;
  for (int i = 0; i < count; i++) {
    if (values[i] > 0) {
      sum = sum + values[i];
    }
  }
  return sum;
}