if ARGV.length != 3
    puts "ruby runner.sh <fib-perf> <load_file> <test_file>"
    exit
end


fib_perf = ARGV[0]
load_file = ARGV[1]
test_file = ARGV[2]

algorithms = ["naive", "cisco", "caesar", "caesar-filter"]

algorithms.each {|alg| 
    output = `#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg}`
    puts output
    
    output_hashed = `#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg} -d`
    puts output_hashed 

    exit
}
