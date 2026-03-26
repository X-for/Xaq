func Hello() -> (ret: String="") {
    out((ret:="Hello World!"))
    return ret
}

out(Hello() )