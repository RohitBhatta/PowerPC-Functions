fun f4(a,b,c,d) {
    print a+b+c+d
}

fun f3(a,b,c) {
    print a+b+c
    x = f4(a,b,c,c+1)
    print a+b+c
}

fun f2(a,b) {
    print(a+b) 
    x = f3(a,b,b+1)
    print(a+b)
}

fun f1(a) {
    print(a)
    x = f2(a,a+1)
    print(a)
}

fun main() {
    x = f1(2)
}

