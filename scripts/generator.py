import sys
import random
import string

def url_to_segments(url):
    segments = []

    if url.startswith("/"):
        url = url[1:]

    current = ""
    index = 0
    for c in url:
        if c == ".":
            segments.append(current)
            current = ""
        elif c == "/":
            segments.append(current)
            current = ""
        elif c == "?":
            segments.append(current)
            segments.append(url[index + 1:])
            break
        else:
            current += c
        index += 1

    return segments

def segments_to_name(segments):
    name = "ccnx:/" + "/".join(segments)
    while name.endswith("/"):
        name = name[:len(name) - 1]
    return name

def url_to_name(url):
    segments = url_to_segments(url)
    return segments_to_name(segments)

def random_string(size=6, chars=string.ascii_uppercase + string.digits):
    return ''.join(random.choice(chars) for _ in range(size))

def extend_name(segments, segment_sampler, segment_length_sampler):
    num_to_add = segment_sampler(len(segments))
    for i in range(num_to_add):
        segments.append(random_string(segment_length_sampler()))
    return segments_to_name(segments)

def generate_load():
    names = []
    for line in sys.stdin:
        if len(line) > 0:
            name = url_to_name(line)
            names.append(name)
    for n in names:
        print n

# Sampled from the unibas data set
NUM_SEGMENTS_MEAN = 5.57
NUM_SEGMENTS_STDEV = 8.14
SEGMENT_LENGTH_MEAN = 10.39
SEGMENT_LENGTH_STDEV = 30.02

def segment_sampler(number):
    global NUM_SEGMENTS_MEAN
    global NUM_SEGMENTS_STDEV

    value = int(random.normalvariate(NUM_SEGMENTS_MEAN, NUM_SEGMENTS_STDEV)) # values from Unibas data set -- fixed for now
    delta = value - number
    count = 0
    while delta < 0:
        count += 1
        value = int(random.normalvariate(NUM_SEGMENTS_MEAN + count, NUM_SEGMENTS_STDEV)) # values from Unibas data set -- fixed for now
        delta = value - number
    return delta

def segment_length_sampler():
    global SEGMENT_LENGTH_MEAN
    global SEGMENT_LENGTH_STDEV

    value = int(random.normalvariate(SEGMENT_LENGTH_MEAN, SEGMENT_LENGTH_STDEV)) # values from Unibas data set -- fixed for now
    count = 0
    while value < 0:
        count += 1
        value = int(random.normalvariate(SEGMENT_LENGTH_MEAN + count, SEGMENT_LENGTH_STDEV)) # values from Unibas data set -- fixed for now
    return value

def generate_test(increase_factor):
    base_names = []
    for line in sys.stdin:
        if len(line) > 0:
            segments = url_to_segments(line)
            base_names.append(segments)

    count = len(base_names)
    total = increase_factor * count

    for i in range(total):
        index = random.randint(0, count - 1)
        name = extend_name(base_names[index], segment_sampler, segment_length_sampler)
        print name

def usage(args):
    print "usage: python %s [load | test] -- " % args[0]
    sys.exit(-1)

def main(args):
    if len(args) < 2:
        usage(args)

    if args[1] == "load":
        generate_load()
    elif args[1] == "test":
        increase_factor = int(args[2])
        generate_test(increase_factor)
    else:
        usage(args)

if __name__ == "__main__":
    main(sys.argv)
