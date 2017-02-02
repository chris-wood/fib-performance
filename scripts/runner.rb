if ARGV.length != 4
    puts "ruby runner.sh <fib-perf> <load_file> <test_file> <suffix>"
    exit
end

fib_perf = ARGV[0]
load_file = ARGV[1]
test_file = ARGV[2]
suffix = ARGV[3]

table_algorithms = ["naive", "cisco"]
filter_algorithms = ["caesar", "caesar-filter", "merged-filter"]
trie_algorithms = ["patricia"]
widths = [4, 8, 12, 16, 20, 24, 28, 32]
numfilters = [2, 3, 4, 5, 6]
filterWidths = [32, 64, 128, 256]

table_algorithms.each {|alg|
    fname = "#{alg}_#{suffix}_out.txt"

    # Run without hashing
    puts "#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg}" 
    output = `#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg}`
    puts output
    File.open(fname, 'a') { |file| file.write(output) }

    # Run with hashing
    widths.each {|width|
        puts "#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg} -d #{width}"
        output_hashed = `#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg} -d #{width}`
        puts output_hashed
        File.open(fname, 'a') { |file| file.write(output_hashed) }
    }
}

filter_algorithms.each {|alg|
    numfilters.each{|k|
        filterWidths.each{|m|
            fname = "#{alg}_#{k}_#{m}_#{suffix}_out.txt"

            puts "#{fib_perf} -l #{load_file} -t #{test_file} -f #{k} -s #{m} -a #{alg}"
            output = `#{fib_perf} -l #{load_file} -t #{test_file} -f #{k} -s #{m} -a #{alg}`
            puts output
            File.open(fname, 'a') { |file| file.write(output) }

            widths.each {|width|
                puts "#{fib_perf} -l #{load_file} -t #{test_file} -d #{width} -f #{k} -s #{m} -a #{alg}"
                output_hashed = `#{fib_perf} -l #{load_file} -t #{test_file} -d #{width} -f #{k} -s #{m} -a #{alg}`
                puts output_hashed
                File.open(fname, 'a') { |file| file.write(output_hashed) }
            }
        }
    }
}

trie_algorithms.each {|alg|
    fname = "#{alg}_#{suffix}_out.txt"

    puts "#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg}" 
    output = `#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg}`
    puts output
    File.open(fname, 'a') { |file| file.write(output) }
}

# Custom FIBs
alg = "tbf"
Ts = [2,3,4,5,6]

Ts.each{|T|
    numfilters.each{|k|
        filterWidths.each{|m|
            fname = "#{alg}_#{suffix}_#{T}_#{k}_#{m}_out.txt"
            puts "#{fib_perf} -l #{load_file} -t #{test_file} -x #{T} -f #{k} -s #{m} -a #{alg}"
            output = `#{fib_perf} -l #{load_file} -t #{test_file} -x #{T} -f #{k} -s #{m} -a #{alg}`
            puts output
            File.open(fname, 'a') { |file| file.write(output) }
        }
    }
}

