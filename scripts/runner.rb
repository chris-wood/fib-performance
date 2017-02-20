if ARGV.length != 4
    STDERR.puts "ruby runner.sh <fib-perf> <load_file> <test_file> <suffix>"
    exit
end

# ruby runner.rb ../b/fib_perf ../data/load.txt ../data/test.txt raven_02182017

fib_perf = ARGV[0]
load_file = ARGV[1]
test_file = ARGV[2]
suffix = ARGV[3]

table_algorithms = ["naive", "cisco"]
filter_algorithms = ["caesar", "caesar-filter", "merged-filter"]
trie_algorithms = ["patricia"]
digestWidths = [4, 8, 16, 32]
numfilters = [2, 3, 4, 5, 6]
filterWidths = [4, 8, 16, 32]

# # Hash-table-based FIBs: Naive, Cisco
# table_algorithms.each {|alg|
#     fname = "#{alg}_#{suffix}_out.txt"
#
#     # Run without hashing
#     STDERR.puts "#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg}"
#     output = `#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg}`
#     STDERR.puts output
#     puts output
#     File.open(fname, 'a') { |file| file.write(output) }
#
#     # Run with hashing
#     digestWidths.each {|width|
#         STDERR.puts "#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg} -d #{width}"
#         output_hashed = `#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg} -d #{width}`
#         STDERR.puts output_hashed
#         puts output_hashed
#         File.open(fname, 'a') { |file| file.write(output_hashed) }
#     }
# }
#
# # Bloom-filter-based FIBs: Caesar, Caesar-filter, Merged-Filter
# filter_algorithms.each {|alg|
#     numfilters.each{|k|
#         filterWidths.each{|m|
#             fname = "#{alg}_#{k}_#{m}_#{suffix}_out.txt"
#
#             STDERR.puts "#{fib_perf} -l #{load_file} -t #{test_file} -f #{k} -s #{m} -a #{alg}"
#             output = `#{fib_perf} -l #{load_file} -t #{test_file} -f #{k} -s #{m} -a #{alg}`
#             STDERR.puts output
#             puts output
#             File.open(fname, 'a') { |file| file.write(output) }
#
#             digestWidths.each {|width|
#                 if width >= m
#                     STDERR.puts "#{fib_perf} -l #{load_file} -t #{test_file} -d #{width} -f #{k} -s #{m} -a #{alg}"
#                     output_hashed = `#{fib_perf} -l #{load_file} -t #{test_file} -d #{width} -f #{k} -s #{m} -a #{alg}`
#                     STDERR.puts output_hashed
#                     puts output_hashed
#                     File.open(fname, 'a') { |file| file.write(output_hashed) }
#                 end
#             }
#         }
#     }
# }
#
# # Patricia FIB
# trie_algorithms.each {|alg|
#     fname = "#{alg}_#{suffix}_out.txt"
#
#     STDERR.puts "#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg}"
#     output = `#{fib_perf} -l #{load_file} -t #{test_file} -a #{alg}`
#     STDERR.puts output
#     puts output
#     File.open(fname, 'a') { |file| file.write(output) }
# }

# TBF FIB
alg = "tbf"
Ts = [2,3,4,5,6]

Ts.each{|t|
    numfilters.each{|k|
        filterWidths.each{|m|
            fname = "#{alg}_#{suffix}_#{t}_#{k}_#{m}_out.txt"
            STDERR.puts "#{fib_perf} -l #{load_file} -t #{test_file} -x #{t} -f #{k} -s #{m} -a #{alg}"
            output = `#{fib_perf} -l #{load_file} -t #{test_file} -x #{t} -f #{k} -s #{m} -a #{alg}`
            STDERR.puts output
            puts output
            File.open(fname, 'a') { |file| file.write(output) }
        }
    }
}
