if ARGV.length != 3
    puts "ruby runner.sh <fib-perf> <load_file> <test_file>"
    exit
end


fib_perf = ARGV[0]
load_file = ARGV[1]
test_file = ARGV[2]

algorithms = ["naive", "cisco"] #, "caesar", "caesar-filter"]
widths = [4, 8, 12, 16, 20, 24, 28, 32]

algorithms.each {|alg| 
    fname = "#{alg}_out.txt"

    output = `#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg}`
    puts output
    File.open(fname, 'a') { |file| file.write(output) }
 
    widths.each {|width|   
        output_hashed = `#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg} -d #{width}`
        puts output_hashed 
        File.open(fname, 'a') { |file| file.write(output_hashed) }
    }

    #exit
}
