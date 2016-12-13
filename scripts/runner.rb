if ARGV.length != 3
    puts "ruby runner.sh <fib-perf> <load_file> <test_file>"
    exit
end


fib_perf = ARGV[0]
load_file = ARGV[1]
test_file = ARGV[2]

table_algorithms = ["naive", "cisco"]
filter_algorithms = [] #"caesar", "caesar-filter", "merged-bf"]
widths = [4, 8, 12, 16, 20, 24, 28, 32]
numfilters = [2, 3, 4, 5, 6]

table_algorithms.each {|alg| 
    fname = "#{alg}_out.txt"

    # Run without hashing
    output = `#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg}`
    puts output
    File.open(fname, 'a') { |file| file.write(output) }
 
    # Run with hashing
    widths.each {|width|   
        output_hashed = `#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg} -d #{width}`
        puts output_hashed 
        File.open(fname, 'a') { |file| file.write(output_hashed) }
    }

    #exit
}

filters_algorithms.each {|alg| 
    numfilters.each{|k| 
        fname = "#{alg}_#{k}_out.txt"

        output = `#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg} -f #{k}`
        puts output
        File.open(fname, 'a') { |file| file.write(output) }

        widths.each {|width|   
            output_hashed = `#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg} -d #{width} -f #{k}`
            puts output_hashed 
            File.open(fname, 'a') { |file| file.write(output_hashed) }
        }
    }
}
