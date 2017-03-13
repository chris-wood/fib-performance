if ARGV.length != 2
    puts "ruby experiment.rb <fib-perf> <base_url>"
    exit
end

attacker = ARGV[0]
url_file = ARGV[1]

# Change these as needed to control the lifetime and accuracy of the experiment
expansion = 5
component_expansion = 10.0
number_expansion = 10.0
num_experiments = 1
fractions = [1].map {|n| n * 0.01}

# Count the number of lines in the file
lines = `wc -l #{url_file}`.split(" ")[0].to_i
counts = fractions.map {|n| (n * lines).to_i}

counts.each{|count|
    load_data = `python generator.py #{count} load < #{url_file}`.split("\n")
    puts "Generated #{count} load data"
    test_data = `python generator.py #{count} test #{expansion} #{component_expansion} #{number_expansion} < #{url_file}`.split("\n")
    puts "Generated #{count} test data"

    (1..num_experiments).each{|n|
        # Shuffle the data
        load_data.shuffle!
        test_data.shuffle!

        tmp_load = "/tmp/load_tmp_#{count}_#{n}.txt"
        tmp_test = "/tmp/test_tmp_#{count}_#{n}.txt"
        attack_out = "attack_#{n}_#{expansion}.txt"

        File.open(tmp_load, 'w') { |file| load_data.each{|line| file.write(line + "\n")} }
        File.open(tmp_test, 'w') { |file| test_data.each{|line| file.write(line + "\n")} }

        suffix = "#{count}_#{n}"
        puts suffix

        out = `#{attacker} #{tmp_load} #{tmp_test}`
        File.open(attack_out, 'w') { |file|
            file.write(out)
        }

        File.delete tmp_load
        File.delete tmp_test
    }
}
