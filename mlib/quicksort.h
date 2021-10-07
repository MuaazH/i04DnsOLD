#ifndef _QUICKSORT_IMPL_H
#define _QUICKSORT_IMPL_H

template<typename T>
void quickSort(T *arr, int left, int right, int (* comparator)(T, T)) {
	int i = left;
	int j = right;
	T tmp;
	T pivot = arr[(left + right) / 2];

	/* partition */
	while (i <= j) {
		// while (arr[i] < pivot)
		while (comparator(arr[i], pivot) < 0)
			i++;

		while (comparator(arr[j], pivot) > 0)
			j--;

		if (i <= j) {
			tmp = arr[i];
			arr[i] = arr[j];
			arr[j] = tmp;
			i++;
			j--;
		}
	}

	/* recursion */
	if (left < j)
		quickSort(arr, left, j, comparator);

	if (i < right)
		quickSort(arr, i, right, comparator);

}
#endif
