// A++ Fibonacci Example
// Author: OXOP

func fibonacci(n: int) -> int {
    if n <= 1 {
        return n
    }
    return fibonacci(n - 1) + fibonacci(n - 2)
}

func fibIterative(n: int) -> int {
    if n <= 1 {
        return n
    }
    let a = 0
    let b = 1
    let i = 2
    while i <= n {
        let temp = a + b
        a = b
        b = temp
        i = i + 1
    }
    return b
}

println("Fibonacci (recursive):")
let i = 0
while i <= 10 {
    print("fib(" + str(i) + ") = ")
    println(str(fibonacci(i)))
    i = i + 1
}

println("")
println("Fibonacci (iterative):")
let j = 0
while j <= 10 {
    print("fib(" + str(j) + ") = ")
    println(str(fibIterative(j)))
    j = j + 1
}
