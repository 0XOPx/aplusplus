// A++ Sorting Algorithms Example
// Author: OXOP

// Bubble Sort
func bubbleSort(arr: list) -> list {
    let n = len(arr)
    let i = 0
    while i < n {
        let j = 0
        while j < n - i - 1 {
            if arr[j] > arr[j + 1] {
                let temp = arr[j]
                arr[j] = arr[j + 1]
                arr[j + 1] = temp
            }
            j = j + 1
        }
        i = i + 1
    }
    return arr
}

// Binary Search
func binarySearch(arr: list, target: int) -> int {
    let low = 0
    let high = len(arr) - 1
    while low <= high {
        let mid = (low + high) / 2
        if arr[mid] == target {
            return mid
        } elif arr[mid] < target {
            low = mid + 1
        } else {
            high = mid - 1
        }
    }
    return -1
}

// Print list helper
func printList(arr: list) {
    print("[")
    let i = 0
    while i < len(arr) {
        if i > 0 { print(", ") }
        print(str(arr[i]))
        i = i + 1
    }
    println("]")
}

// Main
let numbers = [64, 34, 25, 12, 22, 11, 90]
println("Original array:")
printList(numbers)

let sorted = bubbleSort(numbers)
println("Sorted array:")
printList(sorted)

println("")
let target = 25
let result = binarySearch(sorted, target)
if result >= 0 {
    println("Found " + str(target) + " at index " + str(result))
} else {
    println(str(target) + " not found")
}
