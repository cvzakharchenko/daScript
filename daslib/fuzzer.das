options no_unused_block_arguments = false
options no_unused_function_arguments = false
options indenting = 4

module fuzzer shared private

var private total_fuzz_count : int = 1000

def public fuzz ( blk : block )
    //! run block however many times
    //! ignore panic, so that we can see that runtime crashes
    for i in range(total_fuzz_count)
        try
            invoke(blk)
        recover
            pass

def public fuzz ( fuzz_count:int; blk : block )
    //! run block however many times
    //! ignore panic, so that we can see that runtime crashes
    for i in range(fuzz_count)
        try
            invoke(blk)
        recover
            pass

def public fuzz_debug ( blk : block )
    //! run block however many times
    //! do not ignore panic, so that we can see where the runtime fails
    //! this is here so that `fuzz` can be easily replaced with `fuzz_debug` for the purpose of debugging
    for i in range(total_fuzz_count)
        invoke(blk)

def public fuzz_debug ( fuzz_count:int; blk : block )
    //! run block however many times
    //! do not ignore panic, so that we can see where the runtime fails
    //! this is here so that `fuzz` can be easily replaced with `fuzz_debug` for the purpose of debugging
    for i in range(fuzz_count)
        invoke(blk)
