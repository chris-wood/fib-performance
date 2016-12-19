if ARGV.length != 2
    puts "ruby experiment.rb <fib-perf> <base_url>"
    exit
end

fib_perf = ARGV[0]
url_file = ARGV[1]

# Change these as needed
expansion = 5
num_experiments = 1 #100
fractions = (1..10).map {|n| n * 0.1}

# Count the number of lines in the file
lines = `wc -l #{url_file}`.split(" ")[0].to_i
counts = fractions.map {|n| (n * lines).to_i}

counts.each{|count|
    load_data = `python generator.py #{count} load < #{url_file}`.split("\n")
    test_data = `python generator.py #{count} test < #{url_file} #{expansion}`.split("\n")

    (1..num_experiments).each{|n|
        # Shuffle the data 
        load_data.shuffle!
        test_data.shuffle!
    
        tmp_load = "/tmp/load_tmp_#{count}_#{n}.txt"
        tmp_test = "/tmp/test_tmp_#{count}_#{n}.txt"

        File.open(tmp_load, 'w') { |file| load_data.each{|line| file.write(line + "\n")} }
        File.open(tmp_test, 'w') { |file| test_data.each{|line| file.write(line + "\n")} }

        suffix = "#{count}_#{n}"
        puts suffix

        out = `ruby runner.rb #{fib_perf} #{tmp_load} #{tmp_test} #{suffix}`.to_i
        if out != 0
            puts "check it out"
            exit
        end 

        #File.delete tmp_load
        #File.delete tmp_test
    }

    # Only do the first one
    exit
}

